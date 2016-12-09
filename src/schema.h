#ifndef _SNOWFLAKE_SCHEMA
#define _SNOWFLAKE_SCHEMA
#include <Python.h>
#include "structmember.h"


/**
 * Schema.
 *
 * Defines the properties of an ID generation schema.
 */
typedef struct {
    PyObject_HEAD

    /**
     * Number of bits used to represent the timestamp.
     */
    int timestamp_bits;

    /**
     * Number of bits used to represent the data center ID.
     */
    int datacenter_id_bits;

    /**
     * Number of bits used to represent the worker ID.
     */
    int worker_id_bits;

    /**
     * Number of bits used to represent the sequence number.
     */
    int sequence_number_bits;

    /**
     * Epoch.
     *
     * Number of milliseconds since the UNIX epoch.
     */
    PY_LONG_LONG epoch;

    /**
     * Data center ID bits inverted.
     */
    int datacenter_id_bits_inverted;
} Schema;


/**
 * Get the current timestamp as per the schema.
 */
extern PY_LONG_LONG Schema_get_current_timestamp(Schema *self);


extern PyTypeObject SchemaType;
#endif
