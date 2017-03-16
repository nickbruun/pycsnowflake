PY_DIRS := tests

PYTHON_BIN ?= python

all: install test stylecheck

build:
	python setup.py build

install:
	python setup.py install

test:
	nosetests tests

stylecheck:
	flake8 '--exclude=.svn,CVS,.bzr,.hg,.git,__pycache__,.tox,.#*' $(PY_DIRS)

clean:
	find $(PY_DIRS) -type d -name __pycache__ | xargs rm -rf
	rm -rf build

docs:
	cd docs; make html

dist:
	@${PYTHON_BIN} setup.py sdist

.PHONY: all test stylecheck clean dist
