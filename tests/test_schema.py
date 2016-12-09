import unittest

from snowflake import Schema


class SchemaTestCase(unittest.TestCase):
    """Test case for :class:`snowflake.Schema`.
    """

    def test_init(self):
        """snowflake.Schema(..)
        """

        # Test invalid types.
        for value in [0.0, '0']:
            with self.assertRaises(TypeError):
                Schema(timestamp_bits=value)

            with self.assertRaises(TypeError):
                Schema(datacenter_id_bits=value)

            with self.assertRaises(TypeError):
                Schema(worker_id_bits=value)

            with self.assertRaises(TypeError):
                Schema(sequence_number_bits=value)

            with self.assertRaises(TypeError):
                Schema(epoch=value)

        # Test invalid values.
        for timestamp_bits in range(-10, 35):
            with self.assertRaises(ValueError):
                Schema(timestamp_bits=timestamp_bits)

        for bits in range(-10, 0):
            with self.assertRaises(ValueError):
                Schema(datacenter_id_bits=bits)

            with self.assertRaises(ValueError):
                Schema(worker_id_bits=bits)

            with self.assertRaises(ValueError):
                Schema(sequence_number_bits=bits)

        # Test widths over 63 bits.
        with self.assertRaises(ValueError):
            Schema(timestamp_bits=64,
                   datacenter_id_bits=0,
                   worker_id_bits=0,
                   sequence_number_bits=0)

        with self.assertRaises(ValueError):
            Schema(timestamp_bits=50,
                   datacenter_id_bits=4,
                   worker_id_bits=4,
                   sequence_number_bits=6)
