from distutils.core import setup, Extension


setup(
    name='pycsnowflake',
    version='0.1.1',
    description='Fast, flexible-width Twitter Snowflake like unique '
    'ID generation',
    author='Nick Bruun',
    author_email='nick@bruun.co',
    license=open('LICENSE').read(),
    url='https://github.com/nickbruun/pycsnowflake',
    package_data={'': ['LICENSE']},
    ext_modules=[
        Extension(
            'snowflake',
            ['src/module.c', 'src/schema.c', 'src/generator.c'],
            extra_compile_args=['-std=c99'],
            depends=['src/generator.h', 'src/schema.h']
        ),
    ],
    classifiers=(
        'Intended Audience :: Developers',
        'Natural Language :: English',
        'License :: OSI Approved :: Apache Software License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    )
)
