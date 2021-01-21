
#include <QApplication>
#include <QGLViewer/simple_viewer.h>
#include <matrices.h>
#include <primitives.h>
#include <meshquad.h>
#include <omp.h>


const Vec3 ROUGE   = {1,0,0};
const Vec3 VERT    = {0,1,0};
const Vec3 BLEU    = {0,0,1};
const Vec3 JAUNE   = {1,1,0};
const Vec3 CYAN    = {0,1,1};
const Vec3 MAGENTA = {1,0,1};
const Vec3 BLANC   = {1,1,1};
const Vec3 GRIS    = {0.5,0.5,0.5};
const Vec3 NOIR    = {0,0,0};



void draw_repere(const Primitives& prim, const Mat4& tr)
{
    prim.draw_sphere(tr*scale(0.2,0.2,0.2), BLANC);

    prim.draw_cylinder(tr*scale(0.4,0.1,0.1)*translate(0.5,0,0)*rotateY(90),ROUGE);
    prim.draw_cone(tr*scale(0.2,0.2,0.2)*translate(2.5,0,0)*rotateY(90), ROUGE);

    prim.draw_cylinder(tr*scale(0.1,0.4,0.1)*translate(0,0.5,0)*rotateX(90),VERT);
    prim.draw_cone(tr*scale(0.2,0.2,0.2)*translate(0,2.5,0)*rotateX(-90), VERT);

    prim.draw_cylinder(tr*scale(0.1,0.1,0.4)*translate(0,0,0.5),BLEU);
    prim.draw_cone(tr*scale(0.2,0.2,0.2)*translate(0,0,2.5), BLEU);
}


void star(MeshQuad& m)
{
    m.create_cube();
    int size = m.nb_quads();
    for(int i = 0; i < size; i++){
        for(int j = 0; j < 20; j++) {
            m.extrude_quad(i);
            m.decale_quad(i,-glm::sqrt(m.area_of_quad(i))/2);
            m.shrink_quad(i,0.9);
            m.tourne_quad(i,5);
        }
    }
}

void spiral(MeshQuad& m)
{
    m.create_cube();
    int cubes = 200;
    float h = 0.06;
    int i = 0;

    while(m.nb_quads() < cubes*5+1){
        i = i+1;
        if(i%50 == 0) {
            h = h-0.008;
        }
        m.shrink_quad(2,0.995);
        m.transfo_quad(2,rotateX(11)*translate(h, 0, 0));
        m.extrude_quad(2);
    }
}

void abstract_recursive(MeshQuad &m, int q, unsigned long i) {
    if(i<=0)
        return;

    m.extrude_quad(q);
    m.shrink_quad(q,0.5);
    m.extrude_quad(q);
    m.decale_quad(q, glm::sqrt(m.area_of_quad(q)));
    m.extrude_quad(q);
    unsigned long a = m.nb_quads() - 1;
    unsigned long b = a-1;
    unsigned long c = b-1;
    unsigned long d = c-1;

    abstract_recursive(m, a, i-1);
    abstract_recursive(m, b, i-1);
    abstract_recursive(m, c, i-1);
    abstract_recursive(m, d, i-1);
    abstract_recursive(m, q, i-1);
}

void abstract(MeshQuad &m, unsigned long i){
    m.create_cube();
    abstract_recursive(m,4,i);
}

int main(int argc, char *argv[])
{
    Primitives prim;
    int selected_quad = -1;
	glm::mat4 selected_frame;
    MeshQuad mesh;

	// init du viewer
	QApplication a(argc, argv);
	SimpleViewer::init_gl();
	SimpleViewer viewer({0.1,0.1,0.1},5);

	// GL init
	viewer.f_init = [&] ()
	{
		prim.gl_init();
		mesh.gl_init();
	};

	// drawing
	viewer.f_draw = [&] ()
	{
		mesh.set_matrices(viewer.getCurrentModelViewMatrix(),viewer.getCurrentProjectionMatrix());
		prim.set_matrices(viewer.getCurrentModelViewMatrix(),viewer.getCurrentProjectionMatrix());

		mesh.draw(CYAN);

        if (selected_quad>=0)
			draw_repere(prim,selected_frame);
	};

	// to do when key pressed
	viewer.f_keyPress = [&] (int key, Qt::KeyboardModifiers mod)
	{
		switch(key)
		{
			case Qt::Key_C:
                if (!(mod & Qt::ControlModifier)){
                    mesh.create_cube();
                    selected_quad=-1;
                }
				break;
            case Qt::Key_E:
                if (!(mod & Qt::ControlModifier))
                    if(selected_quad != -1){
                        mesh.extrude_quad(selected_quad);
                        selected_quad = -1;
                    }
                break;
            case Qt::Key_Plus: //Shift+Plus

                if(selected_quad != -1){
                    mesh.decale_quad(selected_quad,1);
                    selected_frame = mesh.local_frame(selected_quad);
                }
            break;
            case Qt::Key_Minus: //Minus
                if(selected_quad != -1){
                    mesh.decale_quad(selected_quad,-1);
                    selected_frame = mesh.local_frame(selected_quad);
                }
            break;
            case Qt::Key_Z:
                if (!(mod & Qt::ShiftModifier)){
                    if(selected_quad != -1){
                        mesh.shrink_quad(selected_quad,0.5);
                        selected_frame = mesh.local_frame(selected_quad);
                    }
                } else {
                    if(selected_quad != -1){
                        mesh.shrink_quad(selected_quad,2);
                        selected_frame = mesh.local_frame(selected_quad);
                    }
                }
            break;
            case Qt::Key_T:
                if (!(mod & Qt::ShiftModifier)){
                    if(selected_quad != -1){
                        mesh.tourne_quad(selected_quad,-60);
                        selected_frame = mesh.local_frame(selected_quad);
                    }
                } else {
                    if(selected_quad != -1){
                        mesh.tourne_quad(selected_quad,60);
                        selected_frame = mesh.local_frame(selected_quad);
                    }
                }
            break;

			// e extrusion
			// +/- decale
			// z/Z shrink
			// t/T tourne

			// Attention au cas m_selected_quad == -1

			// generation d'objet
			case Qt::Key_S:
				star(mesh);
				break;
            case Qt::Key_A:
                spiral(mesh);
                break;
            case Qt::Key_R:
                abstract(mesh,3);
            break;
			default:
                break;
        }

		Vec3 sc;
		float r;
		mesh.bounding_sphere(sc,r);
		viewer.setSceneCenter(qglviewer::Vec(sc[0],sc[1],sc[2]));
		viewer.setSceneRadius(r);
		viewer.camera()->centerScene();
		viewer.update();
	};

	// to do when mouse clicked (P + Dir = demi-droite (en espace objet) orthogonale à l'écran passant par le point cliqué
	viewer.f_mousePress3D = [&] (Qt::MouseButton /*b*/, const glm::vec3& P, const glm::vec3& Dir)
	{
        selected_quad = mesh.intersected_closest(P,Dir);
		if (selected_quad>=0)
            selected_frame = mesh.local_frame(selected_quad);
        std::cout <<selected_quad<< std::endl;
	};

    viewer.clearShortcuts();
	viewer.setShortcut(QGLViewer::EXIT_VIEWER,Qt::Key_Escape);
	viewer.show();
	return a.exec();
}
