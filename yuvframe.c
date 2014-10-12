#include <Python.h>
#include <structmember.h>
#include <libavcodec/avcodec.h>

typedef struct {
	PyObject_HEAD

	int width;
	int height;

	int yLineSkip;
	PyObject *yData;

	int uLineSkip;
	PyObject *uData;

	int vLineSkip;
	PyObject *vData;
} h264decode_YUVFrame;

static PyObject *YUVFrame_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	h264decode_YUVFrame *self;
	self = (h264decode_YUVFrame *)type->tp_alloc(type, 0);
	if (self) {
		self->width = 0;
		self->height = 0;

		self->yLineSkip = 0;
		self->yData = PyString_FromString("");
		if (!self->yData) {
			Py_DECREF(self);
			return NULL;
		}
		self->uLineSkip = 0;
		self->uData = PyString_FromString("");
		if (!self->uData) {
			Py_DECREF(self);
			return NULL;
		}
		self->vLineSkip = 0;
		self->vData = PyString_FromString("");
		if (!self->vData) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *)self;
}

static int YUVFrame_init(h264decode_YUVFrame *self, PyObject *args, PyObject *kwds)
{
	return 0;
}

static int YUVFrame_traverse(h264decode_YUVFrame *self, visitproc visit, void *arg)
{
	Py_VISIT(self->yData);
	Py_VISIT(self->uData);
	Py_VISIT(self->vData);
	return 0;
}

static int YUVFrame_clear(h264decode_YUVFrame *self)
{
	Py_CLEAR(self->yData);
	Py_CLEAR(self->uData);
	Py_CLEAR(self->vData);
	return 0;
}

static void YUVFrame_dealloc(h264decode_YUVFrame *self)
{
	YUVFrame_clear(self);
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *YUVFrame_str(h264decode_YUVFrame *self)
{
	return PyString_FromFormat("h264decode.YUVFrame{%dx%d pixels}", self->width, self->height);
}

static PyMemberDef YUVFrame_members[] = {
    {"width", T_INT, offsetof(h264decode_YUVFrame, width), 0, "width of frame in pixels"},
    {"height", T_INT, offsetof(h264decode_YUVFrame, height), 0, "height of frame in pixels"},
    {"yLineSkip", T_INT, offsetof(h264decode_YUVFrame, yLineSkip), 0, "size in bytes of Y picture line"},
    {"yData", T_OBJECT_EX, offsetof(h264decode_YUVFrame, yData), 0, "the raw yData (height lines)"},
    {"uLineSkip", T_INT, offsetof(h264decode_YUVFrame, uLineSkip), 0, "size in bytes of U picture line"},
    {"uData", T_OBJECT_EX, offsetof(h264decode_YUVFrame, uData), 0, "the raw uData (height/2 lines)"},
    {"vLineSkip", T_INT, offsetof(h264decode_YUVFrame, vLineSkip), 0, "size in bytes of V picture line"},
    {"vData", T_OBJECT_EX, offsetof(h264decode_YUVFrame, vData), 0, "the raw vData (height/2 lines)"},
    {NULL}  /* Sentinel */
};

static PyMethodDef YUVFrame_methods[] = {
	{NULL, NULL, 0, NULL}
};

PyTypeObject h264decode_YUVFrameType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "h264decode.YUVFrame",    /*tp_name*/
    sizeof(h264decode_YUVFrame),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)YUVFrame_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    (reprfunc)YUVFrame_str,    /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, /*tp_flags*/
    "A YUV420p frame as returned by x264 decoder in ffmpeg",           /* tp_doc */
    (traverseproc)YUVFrame_traverse,       /* tp_traverse */
    (inquiry)YUVFrame_clear,  /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    YUVFrame_methods,          /* tp_methods */
    YUVFrame_members,          /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)YUVFrame_init,   /* tp_init */
    0,                         /* tp_alloc */
    YUVFrame_new,              /* tp_new */
};

PyObject *h264decode_YUVFrame_from_AVPicture(AVFrame *picture)
{
	h264decode_YUVFrame *self = (h264decode_YUVFrame *)
					PyObject_CallObject((PyObject *) &h264decode_YUVFrameType, NULL);

	self->width = picture->width;
	self->height = picture->height;
	self->yLineSkip = picture->linesize[0];
	self->uLineSkip = picture->linesize[1];
	self->vLineSkip = picture->linesize[2];

	Py_CLEAR(self->yData);
	self->yData = Py_BuildValue("s#", picture->data[0], picture->height * picture->linesize[0]);
	if (!self->yData) {
		Py_DECREF(self);
		return NULL;
	}

	Py_CLEAR(self->uData);
	self->uData = Py_BuildValue("s#", picture->data[1], picture->height / 2 * picture->linesize[1]);
	if (!self->uData) {
		Py_DECREF(self);
		return NULL;
	}

	Py_CLEAR(self->vData);
	self->vData = Py_BuildValue("s#", picture->data[2], picture->height / 2 * picture->linesize[2]);
	if (!self->vData) {
		Py_DECREF(self);
		return NULL;
	}

	return (PyObject  *)self;
}
