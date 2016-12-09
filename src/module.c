#include <Python.h>

#include "generator.h"
#include "schema.h"


static PyModuleDef snowflakemodule = {
    PyModuleDef_HEAD_INIT,
    "snowflake",
    "Flexible implementation of the basic idea of Twitter Snowflake.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};


PyMODINIT_FUNC
PyInit_snowflake(void)
{
    PyObject* m;

    if (PyType_Ready(&SchemaType) < 0)
        return NULL;

    if (PyType_Ready(&GeneratorType) < 0)
        return NULL;

    m = PyModule_Create(&snowflakemodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&SchemaType);
    PyModule_AddObject(m, "Schema", (PyObject *)&SchemaType);

    Py_INCREF(&GeneratorType);
    PyModule_AddObject(m, "Generator", (PyObject *)&GeneratorType);

    return m;
}
