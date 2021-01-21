TEMPLATE = subdirs

SUBDIRS = QGLViewer OGLRender Projet_modeling

 # what subproject depends on others
Projet_modeling.depends = QGLViewer OGLRender
