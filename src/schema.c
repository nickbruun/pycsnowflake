#include <sys/time.h>

#include "schema.h"


static void Schema_dealloc(Schema* self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyObject *Schema_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Schema *self;

    self = (Schema *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->timestamp_bits = 41;
        self->datacenter_id_bits = 5;
        self->worker_id_bits = 5;
        self->sequence_number_bits = 12;
        self->epoch = 1288834974657ll;
    }

    return (PyObject *)self;
}

static int Schema_init(Schema *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {
        "timestamp_bits",
        "datacenter_id_bits",
        "worker_id_bits",
        "sequence_number_bits",
        "epoch",
        "datacenter_id_bits_inverted",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwds, "|iiiiLp",
                                     kwlist,
                                     &self->timestamp_bits,
                                     &self->datacenter_id_bits,
                                     &self->worker_id_bits,
                                     &self->sequence_number_bits,
                                     &self->epoch,
                                     &self->datacenter_id_bits_inverted)) {
        return -1;
    }

    if (self->timestamp_bits < 35) {
        PyErr_SetString(PyExc_ValueError, "At least 35 bits must be allocated for the timestamp");
        return -1;
    }

    if (self->datacenter_id_bits < 0) {
        PyErr_SetString(PyExc_ValueError, "At least 0 bits must be allocated for the data center ID");
        return -1;
    }

    if (self->worker_id_bits < 0) {
        PyErr_SetString(PyExc_ValueError, "At least 0 bits must be allocated for the worker ID");
        return -1;
    }

    if (self->sequence_number_bits < 0) {
        PyErr_SetString(PyExc_ValueError, "At least 0 bits must be allocated for the sequence number");
        return -1;
    }

    if (self->timestamp_bits +
        self->datacenter_id_bits +
        self->worker_id_bits +
        self->sequence_number_bits > 63) {
        PyErr_SetString(PyExc_ValueError, "The sum of bits making up all fields must not exceed 63 bits");
        return -1;
    }

    return 0;
}


PY_LONG_LONG Schema_get_current_timestamp(Schema *self) {
    struct timeval current_time;
    /* TODO: error handling? */
    gettimeofday(&current_time, NULL);

    return current_time.tv_sec * 1000 + current_time.tv_usec / 1000 - self->epoch;
}


static PyMemberDef Schema_members[] = {
    {NULL},
};


#define SNOWFLAKE_PY_FIELD_GETTERS(attr)                                \
    static PyObject *Schema_ ## attr ## _bits_getter(Schema *self, void *closure) { \
        return PyLong_FromLong(self->attr ## _bits);                    \
    }                                                                   \
                                                                        \
    static PyObject *Schema_ ## attr ## _max_getter(Schema *self, void *closure) { \
    return PyLong_FromLong((1 << self->attr ## _bits) - 1);             \
    }

SNOWFLAKE_PY_FIELD_GETTERS(timestamp)
SNOWFLAKE_PY_FIELD_GETTERS(datacenter_id)
SNOWFLAKE_PY_FIELD_GETTERS(worker_id)
SNOWFLAKE_PY_FIELD_GETTERS(sequence_number)

#undef SNOWFLAKE_PY_FIELD_GETTERS


static PyObject *Schema_epoch_getter(Schema *self, void *closure)
{
    return PyLong_FromLongLong(self->epoch);
}


static PyObject *Schema_datacenter_id_bits_inverted_getter(Schema *self, void *closure)
{
    if (self->datacenter_id_bits_inverted)
        Py_RETURN_TRUE;
    else
        Py_RETURN_FALSE;
}


static PyGetSetDef Schema_getseters[] = {
    {"timestamp_bits",
     (getter)Schema_timestamp_bits_getter,
     NULL,
     "Timestamp bits.",
     NULL},

    {"datacenter_id_bits",
     (getter)Schema_datacenter_id_bits_getter,
     NULL,
     "Data center ID bits.",
     NULL},

    {"worker_id_bits",
     (getter)Schema_worker_id_bits_getter,
     NULL,
     "Worker ID bits.",
     NULL},

    {"sequence_number_bits",
     (getter)Schema_sequence_number_bits_getter,
     NULL,
     "Sequence number bits.",
     NULL},

    {"timestamp_max",
     (getter)Schema_timestamp_max_getter,
     NULL,
     "Maximum timestamp.",
     NULL},

    {"datacenter_id_max",
     (getter)Schema_datacenter_id_max_getter,
     NULL,
     "Maximum data center ID.",
     NULL},

    {"worker_id_max",
     (getter)Schema_worker_id_max_getter,
     NULL,
     "Maximum worker ID.",
     NULL},

    {"sequence_number_max",
     (getter)Schema_sequence_number_max_getter,
     NULL,
     "Maximum sequence number.",
     NULL},

    {"epoch",
     (getter)Schema_epoch_getter,
     NULL,
     "Epoch.",
     NULL},

    {"datacenter_id_bits_inverted",
     (getter)Schema_datacenter_id_bits_inverted_getter,
     NULL,
     "Whether data center ID bits should be inverted.",
     NULL},

    {NULL},
};


static PyMethodDef Schema_methods[] = {
    {NULL},
};


PyTypeObject SchemaType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "snowflake.Schema",         /* tp_name */
    sizeof(Schema),             /* tp_basicsize */
    0,                          /* tp_itemsize */
    (destructor)Schema_dealloc, /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    0,                          /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    0,                          /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,    /* tp_flags */
    "ID generation schema",     /* tp_doc */
    0,                          /* tp_traverse */
    0,                          /* tp_clear */
    0,                          /* tp_richcompare */
    0,                          /* tp_weaklistoffset */
    0,                          /* tp_iter */
    0,                          /* tp_iternext */
    Schema_methods,             /* tp_methods */
    Schema_members,             /* tp_members */
    Schema_getseters,           /* tp_getset */
    0,                          /* tp_base */
    0,                          /* tp_dict */
    0,                          /* tp_descr_get */
    0,                          /* tp_descr_set */
    0,                          /* tp_dictoffset */
    (initproc)Schema_init,      /* tp_init */
    0,                          /* tp_alloc */
    Schema_new,                 /* tp_new */
};
