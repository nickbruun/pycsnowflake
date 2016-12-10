import datetime
import unittest

from snowflake import Generator, Schema


ID_SCHEMA = Schema(
    datacenter_id_bits=3,
    worker_id_bits=7,
    epoch=1466128800000,
    datacenter_id_bits_inverted=True
)
"""ID schema.
"""


def get_current_timestamp():
    return int(
        (datetime.datetime.utcnow() -
         datetime.datetime(2016, 6, 17, 2, 0, 0)).total_seconds() * 1000
    )


def split_id(id):
    sequence_number = id & ((1 << 12) - 1)
    worker_id = (id >> 12) & ((1 << 7) - 1)
    datacenter_id = {
        0b000: 0b000,
        0b001: 0b100,
        0b010: 0b010,
        0b011: 0b110,
        0b100: 0b001,
        0b101: 0b101,
        0b110: 0b011,
        0b111: 0b111,
    }[(id >> 19) & ((1 << 3) - 1)]
    timestamp = id >> 22

    return (timestamp, datacenter_id, worker_id, sequence_number)


class GeneratorTestCase(unittest.TestCase):
    """Test case for :class:`snowflake.Generator`.
    """

    def test_generate(self):
        """snowflake.Generator(..).generate()
        """

        # Test data center and worker IDs.
        for datacenter_id in range(2 ** 3):
            for worker_id in range(2 ** 7):
                generator = Generator(schema=ID_SCHEMA,
                                      datacenter_id=datacenter_id,
                                      worker_id=worker_id)

                generated_at = get_current_timestamp()
                id = generator.generate()

                (id_timestamp, id_datacenter_id, id_worker_id,
                 id_sequence_number) = split_id(id)

                self.assertGreaterEqual(id_timestamp, generated_at)
                self.assertLess(id_timestamp, generated_at + 100)
                self.assertEqual(0, id_sequence_number)
                self.assertEqual(id_worker_id, worker_id)
                self.assertEqual(id_datacenter_id, datacenter_id)

        # Test generation of monotonically increasing IDs.
        generator = Generator(schema=ID_SCHEMA,
                              datacenter_id=3,
                              worker_id=45)
        id_prev = generator.generate()

        for i in range(10000):
            id = generator.generate()

            self.assert_id_progression(id, id_prev)

            id_prev = id

    def test_generate_many(self):
        """snowflake.Generator(..).generate_many()
        """

        # Test data center and worker IDs.
        for datacenter_id in range(2 ** 3):
            for worker_id in range(2 ** 7):
                generator = Generator(schema=ID_SCHEMA,
                                      datacenter_id=datacenter_id,
                                      worker_id=worker_id)

                generated_at = get_current_timestamp()
                id = (generator.generate_many(1))[0]

                (id_timestamp, id_datacenter_id, id_worker_id,
                 id_sequence_number) = split_id(id)

                self.assertGreaterEqual(id_timestamp, generated_at)
                self.assertLess(id_timestamp, generated_at + 100)
                self.assertEqual(0, id_sequence_number)
                self.assertEqual(id_worker_id, worker_id)
                self.assertEqual(id_datacenter_id, datacenter_id)

        # Test generation of monotonically increasing IDs.
        generator = Generator(schema=ID_SCHEMA,
                              datacenter_id=3,
                              worker_id=45)
        ids = generator.generate_many(10000)

        for i in range(1, len(ids)):
            self.assert_id_progression(ids[i], ids[i - 1])

    def assert_id_progression(self, cur, prev):
        self.assertGreater(cur, prev)

        (timestamp_cur, datacenter_id_cur, worker_id_cur,
         sequence_number_cur) = split_id(cur)

        (timestamp_prev, datacenter_id_prev, worker_id_prev,
         sequence_number_prev) = split_id(prev)

        self.assertEqual(datacenter_id_cur, datacenter_id_prev)
        self.assertEqual(worker_id_cur, worker_id_prev)

        if timestamp_cur == timestamp_prev:
            self.assertGreater(sequence_number_cur, sequence_number_prev)
        else:
            self.assertGreater(timestamp_cur, timestamp_prev)
            self.assertEqual(sequence_number_cur, 0)
