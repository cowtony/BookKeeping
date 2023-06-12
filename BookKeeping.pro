TEMPLATE = subdirs

SUBDIRS = \
    app \
    test

test.depends = app
