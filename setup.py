from distutils.core import setup, Extension


setup(
    name='snowflake',
    version='0.1.0',
    ext_modules=[
        Extension(
            'snowflake',
            ['src/module.c', 'src/schema.c', 'src/generator.c'],
            extra_compile_args=['-std=c99']
        ),
    ]
)
