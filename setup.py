from distutils.core import setup, Extension


setup(
    name='snowflake',
    version='0.1.0',
    ext_modules=[
        Extension(
            'snowflake',
            ['src/module.c', 'src/schema.c', 'src/generator.c']
        ),
    ],
    tests_require=[
        'nose>=1.3.7,<2.0.0'
    ]
)
