#include "generator.h"
#include "schema.h"


/**
 * Generator.
 *
 * Defines the properties of an ID generation generator.
 */
typedef struct {
    PyObject_HEAD

    /**
     * Schema.
     */
    Schema *schema;

    /**
     * Data center ID.
     */
    int datacenter_id;

    /**
     * Worker ID.
     */
    int worker_id;

    /**
     * Timestamp mask.
     */
    PY_LONG_LONG timestamp_mask;

    /**
     * Timestamp offset.
     */
    int timestamp_offset;

    /**
     * Identity ID part.
     *
     * Shifted data center and worker ID part to include in all IDs.
     */
    PY_LONG_LONG identity_id_part;

    /**
     * Maximum value of the sequence number.
     */
    PY_LONG_LONG sequence_number_max;

    /**
     * Last timestamp.
     */
    PY_LONG_LONG last_timestamp;

    /**
     * Last sequence number.
     */
    PY_LONG_LONG last_sequence_number;
} Generator;


static void Generator_dealloc(Generator *self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyObject *Generator_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Generator *self;

    if ((self = (Generator *)type->tp_alloc(type, 0))) {
        self->schema = NULL;
    }

    return (PyObject *)self;
}


static int Generator_init(Generator *self, PyObject *args, PyObject *kwds)
{
    /* Parse arguments. */
    static char *kwlist[] = {
        "schema",
        "datacenter_id",
        "worker_id",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwds, "O!ii",
                                     kwlist,
                                     &SchemaType,
                                     &self->schema,
                                     &self->datacenter_id,
                                     &self->worker_id)) {
        return -1;
    }

    int datacenter_id_max = (1 << self->schema->datacenter_id_bits) - 1;

    if ((self->datacenter_id < 0) || (self->datacenter_id > datacenter_id_max)) {
        PyErr_Format(PyExc_ValueError, "Data center ID must be between 0 and %d: %d", datacenter_id_max, self->datacenter_id);
        return -1;
    }

    int worker_id_max = (1 << self->schema->worker_id_bits) - 1;

    if ((self->worker_id < 0) || (self->worker_id > worker_id_max)) {
        PyErr_Format(PyExc_ValueError, "Worker ID must be between 0 and %d: %d", worker_id_max, self->worker_id);
        return -1;
    }

    /* Set up generator. */
    Py_INCREF(self->schema);

    self->timestamp_mask = (1ll << self->schema->timestamp_bits) - 1;
    self->timestamp_offset = self->schema->sequence_number_bits + self->schema->worker_id_bits + self->schema->datacenter_id_bits;
    self->sequence_number_max = (1ll << self->schema->sequence_number_bits) - 1;
    self->last_timestamp = Schema_get_current_timestamp(self->schema);
    self->last_sequence_number = -1;

    int identity_datacenter_id;

    if (self->schema->datacenter_id_bits_inverted) {
        identity_datacenter_id = 0;

        for (int i = 0; i < self->schema->datacenter_id_bits; ++i) {
            if (self->datacenter_id & (1 << i)) {
                identity_datacenter_id |= 1 << (self->schema->datacenter_id_bits - 1 - i);
            }
        }
    }
    else {
        identity_datacenter_id = self->datacenter_id;
    }

    self->identity_id_part = ((identity_datacenter_id << self->schema->worker_id_bits) | self->worker_id) << self->schema->sequence_number_bits;

    return 0;
}


static PyMemberDef Generator_members[] = {
    {NULL},
};


static PyObject *Generator_datacenter_id_getter(Generator *self, void *closure)
{
    return PyLong_FromLongLong(self->datacenter_id);
}


static PyObject *Generator_worker_id_getter(Generator *self, void *closure)
{
    return PyLong_FromLongLong(self->worker_id);
}


static PyObject *Generator_schema_getter(Generator *self, void *closure)
{
    Py_INCREF(self->schema);
    return (PyObject *)self->schema;
}


static PyGetSetDef Generator_getseters[] = {
    {"schema",
     (getter)Generator_schema_getter,
     NULL,
     "Schema.",
     NULL},

    {"datacenter_id",
     (getter)Generator_datacenter_id_getter,
     NULL,
     "Data center ID.",
     NULL},

    {"worker_id",
     (getter)Generator_worker_id_getter,
     NULL,
     "Worker ID.",
     NULL},

    {NULL},
};


/**
 * Generate one ID synchronously.
 *
 * @returns the generated ID.
 */
static PY_LONG_LONG Generator_generate_one_sync(Generator *self)
{
    while (1) {
        /* Get the current timestamp and handle changes. */
        PY_LONG_LONG timestamp = Schema_get_current_timestamp(self->schema);

        if (timestamp > self->last_timestamp) {
            self->last_timestamp = timestamp;
            self->last_sequence_number = -1;
        }
        else if (timestamp < self->last_timestamp) {
            /* Spin for now. */
            continue;
        }

        /* Increment the sequence number. */
        if (self->last_sequence_number >= self->sequence_number_max) {
            /* Spin for now. */
            continue;
        }

        PY_LONG_LONG sequence = (++self->last_sequence_number);

        return (
            (timestamp << self->timestamp_offset) |
            self->identity_id_part |
            sequence
        );
    }
}


static PyObject *Generator_generate(Generator *self)
{
    return PyLong_FromLongLong(Generator_generate_one_sync(self));
}


static PyObject *Generator_generate_many(Generator *self, PyObject *args, PyObject *kwds)
{
    /* Parse arguments. */
    static char *kwlist[] = {
        "count",
        NULL
    };
    Py_ssize_t count = 0;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwds,
                                     "n",
                                     kwlist,
                                     &count)) {
        return NULL;
    }

    if (count < 0) {
        PyErr_Format(PyExc_ValueError, "The number of IDs to generate cannot be negative: %d", count);
        return NULL;
    }

    /* Allocate the result list. */
    PyObject *result = PyList_New(count);

    if (!result) {
        return NULL;
    }

    /* Generate IDs. */
    for (Py_ssize_t i = 0; i < count; ++i) {
        PyList_SetItem(result, i, PyLong_FromLongLong(Generator_generate_one_sync(self)));
    }

    return result;
}


static PyMethodDef Generator_methods[] = {
    {"generate",
     (PyCFunction)Generator_generate,
     METH_NOARGS,
     "Generate an ID"},
    {"generate_many",
     (PyCFunction)Generator_generate_many,
     METH_VARARGS | METH_KEYWORDS,
     "Generate multiple IDs"},
    {NULL},
};


PyTypeObject GeneratorType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "snowflake.Generator",         /* tp_name */
    sizeof(Generator),             /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)Generator_dealloc, /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_reserved */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash  */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "ID generator",                /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    Generator_methods,             /* tp_methods */
    Generator_members,             /* tp_members */
    Generator_getseters,           /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    (initproc)Generator_init,      /* tp_init */
    0,                             /* tp_alloc */
    Generator_new,                 /* tp_new */
};
