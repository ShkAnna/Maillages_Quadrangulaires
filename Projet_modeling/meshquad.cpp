#include "meshquad.h"
#include "matrices.h"


void MeshQuad::clear()
{
    m_points.clear();
    m_quad_indices.clear();
}

int MeshQuad::add_vertex(const Vec3& P)
{
    m_points.push_back(P);
    size_t pos = m_points.size() - 1;
    return pos;
}


void MeshQuad::add_quad(int i1, int i2, int i3, int i4)
{
    m_quad_indices.push_back(i1);
    m_quad_indices.push_back(i2);
    m_quad_indices.push_back(i3);
    m_quad_indices.push_back(i4);
}

void MeshQuad::convert_quads_to_tris(const std::vector<int>& quads, std::vector<int>& tris)
{
    tris.clear();
    tris.reserve(3*quads.size()/2);

    for(int i = 0; i < quads.size(); i=i+4)
    {
        tris.push_back(quads[i]);
        tris.push_back(quads[i+1]);
        tris.push_back(quads[i+2]);

        tris.push_back(quads[i+2]);
        tris.push_back(quads[i+3]);
        tris.push_back(quads[i]);
    }
}
bool neighbourHave(int i, const std::vector<int>& quads, std::vector<int>& edges)
{
    int next = (i+1)%4;

     for(int j = 0; j < edges.size(); j++){
         if(next == 0){
            if(edges[j] == quads[int(i/4)*4] && edges[j+1] == quads[i]){
                 return true;
            }
         } else {
            if(edges[j] == quads[i+1] && edges[j+1] == quads[i]){
                return true;
            }
         }
     }
     return false;
}

void MeshQuad::convert_quads_to_edges(const std::vector<int>& quads, std::vector<int>& edges)
{
    edges.clear();
    edges.reserve(quads.size()); // ( *2 /2 !)
    // Pour chaque quad on genere 4 aretes, 1 arete = 2 indices.
    // Mais chaque arete est commune a 2 quads voisins !
    // Comment n'avoir qu'une seule fois chaque arete ?

    for(int i = 0; i < quads.size(); i+=4)
    {
        //On vérifie que l'arête inverse n'est pas déjà présente
        // Avant de l'ajouter à edges

        if(!neighbourHave(i, quads, edges)){
            edges.push_back(quads[i]);
            edges.push_back(quads[i+1]);
        }

        if(!neighbourHave(i+1, quads, edges)){
            edges.push_back(quads[i+1]);
            edges.push_back(quads[i+2]);
        }

        if(!neighbourHave(i+2, quads, edges)){
            edges.push_back(quads[i+2]);
            edges.push_back(quads[i+3]);
        }
        if(!neighbourHave(i+3, quads, edges)){
            edges.push_back(quads[i+3]);
            edges.push_back(quads[i]);
        }
    }
}


void MeshQuad::bounding_sphere(Vec3& C, float& R)
{
    Vec3 min = Vec3(0,0,0);
    Vec3 max = Vec3(0,0,0);

    for(int i=0 ; i < m_points.size(); i++) {
        if (m_points[i].x < min.x)
            min.x = m_points[i].x;
        if (m_points[i].x > max.x)
            max.x = m_points[i].x;
        if (m_points[i].y < min.y)
            min.y = m_points[i].y;
        if (m_points[i].y > max.y)
            max.y = m_points[i].y;
        if (m_points[i].z < min.z)
            min.z = m_points[i].z;
        if (m_points[i].z > max.z)
            max.z = m_points[i].z;
    }
    C = Vec3((min.x+max.x)/2,(min.y+max.y)/2,(min.z+max.z)/2);

    max =Vec3(abs(max.x-C.x),abs(max.y-C.y),abs(max.z-C.z));

    R = max.x;
    if (R < max.y)
        R = max.y;
    if (R < max.z)
        R = max.z;
}


void MeshQuad::create_cube()
{
    clear();
    // ajouter 8 sommets (-1 +1)
    int p1 = add_vertex(Vec3(0,0,0));
    int p2 = add_vertex(Vec3(1,0,0));
    int p3 = add_vertex(Vec3(1,1,0));
    int p4 = add_vertex(Vec3(0,1,0));
    int p5 = add_vertex(Vec3(0,0,1));
    int p6 = add_vertex(Vec3(1,0,1));
    int p7 = add_vertex(Vec3(1,1,1));
    int p8 = add_vertex(Vec3(0,1,1));

    add_quad(p1,p4,p3,p2);
    add_quad(p1,p2,p6,p5);
    add_quad(p2,p3,p7,p6);
    add_quad(p1,p5,p8,p4);
    add_quad(p4,p8,p7,p3);
    add_quad(p5,p6,p7,p8);
    gl_update();
}

Vec3 MeshQuad::normal_of(const Vec3& A, const Vec3& B, const Vec3& C)
{
    // Attention a l'ordre des points !
    // le produit vectoriel n'est pas commutatif U ^ V = - V ^ U
    // ne pas oublier de normaliser le resultat.

    Vec3 AB = Vec3(B.x-A.x,B.y-A.y,B.z-A.z);
    Vec3 AC = Vec3(C.x-A.x,C.y-A.y,C.z-A.z);
    Vec3 prodVect = glm::cross(AB,AC);
    float leng = glm::length(prodVect);
    return Vec3(prodVect.x/leng, prodVect.y/leng, prodVect.z/leng);
}

float distancePandVect(const Vec3& P, const Vec3& V1, const Vec3& V2, Vec3 normal)
{
    Vec3 vecV1V2 = Vec3(V2.x-V1.x, V2.y-V1.y, V2.z-V1.z);
    Vec3 normal_plan = glm::cross(normal,vecV1V2);//Vec3(vecV1V2.x-normal.x,vecV1V2.y-normal.y,vecV1V2.z-normal.z);//
    Vec3 vecV1P = Vec3(P.x-V1.x, P.y-V1.y, P.z-V1.z);

    return glm::dot(normal_plan, vecV1P)/glm::length(normal_plan);
}

bool MeshQuad::is_points_in_quad(const Vec3& P, const Vec3& A, const Vec3& B, const Vec3& C, const Vec3& D)
{
    // On sait que P est dans le plan du quad.

    // P est-il au dessus des 4 plans contenant chacun la normale au quad et une arete AB/BC/CD/DA ?
    // si oui il est dans le quad

    Vec3 normal = normal_of(A,B,C);

    if(distancePandVect(P,A,B, normal) < 0 || distancePandVect(P,B,C, normal) < 0 ||
       distancePandVect(P,C,D, normal) < 0 || distancePandVect(P,D,A, normal) < 0)
         return false;
    else return true;
}

bool MeshQuad::intersect_ray_quad(const Vec3& P, const Vec3& Dir, int q, Vec3& inter)
{
    // recuperation des indices de points
    // recuperation des points

    // calcul de l'equation du plan (N+d)

    // calcul de l'intersection rayon plan
    // I = P + alpha*Dir est dans le plan => calcul de alpha

    // alpha => calcul de I

    // I dans le quad ?
    Vec3 A = m_points[m_quad_indices[q]];
    Vec3 B = m_points[m_quad_indices[q+1]];
    Vec3 C = m_points[m_quad_indices[q+2]];
    Vec3 D = m_points[m_quad_indices[q+3]];
    Vec3 normalPlane = normal_of(A,B,C);

    float a = glm::dot(normalPlane, Dir);
    if(a == 0) {
        return false;
    }
    float t = glm::dot(normalPlane,(A - P))/ a;
    if(t < 0) {
        return false;
    } else {
        float x = P.x + t * Dir.x;
        float y = P.y + t * Dir.y;
        float z = P.z + t * Dir.z;
        inter = Vec3(x,y,z);
        is_points_in_quad(inter, A, B, C, D);
    }
}


int MeshQuad::intersected_closest(const Vec3& P, const Vec3& Dir)
{
    // on parcours tous les quads
    // on teste si il y a intersection avec le rayon
    // on garde le plus proche (de P)
    Vec3 intersect;
    float distCur;
    float distPrec;
    int inter = -1;
    for(int i = 0; i < m_quad_indices.size(); i=i+4){
        if(intersect_ray_quad(P,Dir, i, intersect)){
            distCur = glm::distance(intersect,P);
            if (inter == -1) {
                distPrec = glm::distance(intersect,P);
                inter = i/4;
            } else {
                if(distCur < distPrec){
                    inter = i/4;
                 }
            }
            distPrec = distCur;
        }
    }
    return inter;
}


Mat4 MeshQuad::local_frame(int q)
{
    // Repere locale = Matrice de transfo avec
    // les trois premieres colones: X,Y,Z locaux
    // la derniere colonne l'origine du repere

    // ici Z = N et X = AB
    // Origine le centre de la face
    // longueur des axes : [AB]/2

    // recuperation des indices de points
    // recuperation des points

    // calcul de Z:N / X:AB -> Y

    // calcul du centre

    // calcul de la taille

    // calcul de la matrice

    Vec3 A = getVector(q,0);
    Vec3 B = getVector(q,1);
    Vec3 C = getVector(q,2);
    Vec3 D = getVector(q,3);

    Vec3 nABD = normal_of(A,B,D);
    Vec3 nBCD = normal_of(B,C,D);
    Vec3 Z = Vec3((nABD.x + nBCD.x)/2, (nABD.y + nBCD.y)/2, (nABD.z + nBCD.z)/2);
    Vec3 X = glm::normalize(Vec3(B.x-A.x,B.y-A.y,B.z-A.z));
    Vec3 Y = glm::cross(Z,X);

    float size = glm::sqrt(glm::length(Vec3(B.x-A.x,B.y-A.y,B.z-A.z)))/2;

    Vec3 origin = Vec3((A.x+C.x)/2,(A.y+C.y)/2,(A.z+C.z)/2);
    Mat4 transf = Mat4(X.x,X.y,X.z,0,Y.x,Y.y,Y.z,0,Z.x,Z.y,Z.z,0,origin.x,origin.y,origin.z,1);
    transf = size*transf;
    return transf;
}

float MeshQuad::area_of_quad(int q){
    float area;
    Vec3 A = getVector(q,0);
    Vec3 B = getVector(q,1);
    Vec3 C = getVector(q,2);
    Vec3 D = getVector(q,3);

    float AB = glm::abs(glm::distance(A,B));//Vec3(B.x-A.x,B.y-A.y,B.z-A.z);
    float BC = glm::abs(glm::distance(B,C));//Vec3(C.x-B.x,C.y-B.y,C.z-B.z);
    float CD = glm::abs(glm::distance(C,D));//Vec3(D.x-C.x,D.y-C.y,D.z-C.z);
    float DA = glm::abs(glm::distance(D,A));//Vec3(A.x-D.x,A.y-D.y,A.z-D.z);
    float s = ( AB + BC + CD + DA) / 2;
    area = (s - AB) * (s - BC) * (s - CD) * (s - DA);

    return area;
}

Vec3 MeshQuad::getVector(int q, int id) {
    return m_points[m_quad_indices[q*4+id]];
}

void MeshQuad::extrude_quad(int q)
{
    // recuperation des indices de points

    // recuperation des points

    // calcul de la normale

    // calcul de la hauteur

    // calcul et ajout des 4 nouveaux points

    // on remplace le quad initial par le quad du dessu

    // on ajoute les 4 quads des cotes
    unsigned long i = q*4;

    unsigned long prec1 = m_quad_indices[i];
    unsigned long prec2 = m_quad_indices[i+1];
    unsigned long prec3 = m_quad_indices[i+2];
    unsigned long prec4 = m_quad_indices[i+3];

    Vec3 A = m_points[prec1];
    Vec3 B = m_points[prec2];
    Vec3 C = m_points[prec3];
    Vec3 D = m_points[prec4];
    Vec3 nABD = normal_of(A, B, D);
    Vec3 nBCD = normal_of(B, C, D);
    Vec3 normal = glm::normalize(Vec3((nABD.x + nBCD.x)/2, (nABD.y + nBCD.y)/2, (nABD.z + nBCD.z)/2));

    // calcul de la hauteur
    float Ha = glm::sqrt(area_of_quad(q))/glm::abs(glm::distance(A,B)); //glm::sqrt(area_of_quad(q));//glm::sqrt(area_of_quad(q));//glm::distance(A,B);//

    // calcul et ajout des 4 nouveaux points
    Vec3 na = {normal.x * Ha, normal.y * Ha, normal.z * Ha};


    unsigned long p1 = add_vertex(A + na);
    unsigned long p2 = add_vertex(B + na);
    unsigned long p3 = add_vertex(C + na);
    unsigned long p4 = add_vertex(D + na);

    m_quad_indices[i] = p1;
    m_quad_indices[i+1] = p2;
    m_quad_indices[i+2] = p3;
    m_quad_indices[i+3] = p4;

    add_quad(prec1,prec2,p2,p1);
    add_quad(prec2,prec3,p3,p2);
    add_quad(prec3,prec4,p4,p3);
    add_quad(prec4,prec1,p1,p4);

   gl_update();
}

void MeshQuad::transfo_quad(int q, const glm::mat4& tr)
{
    // recuperation des indices de points
    // recuperation des (references de) points

    // generation de la matrice de transfo globale:
    // indice utilisation de glm::inverse() et de local_frame

    // Application au 4 points du quad
    unsigned long i = q*4;
    Vec3 A = m_points[m_quad_indices[i]];
    Vec3 B = m_points[m_quad_indices[i+1]];
    Vec3 C = m_points[m_quad_indices[i+2]];
    Vec3 D = m_points[m_quad_indices[i+3]];

    Mat4 model = local_frame(q)*tr*glm::inverse(local_frame(q));

    m_points[m_quad_indices[i]] = Vec3(model*Vec4(A.x,A.y,A.z,1));
    m_points[m_quad_indices[i+1]] = Vec3(model*Vec4(B.x,B.y,B.z,1));
    m_points[m_quad_indices[i+2]] = Vec3(model*Vec4(C.x,C.y,C.z,1));
    m_points[m_quad_indices[i+3]] = Vec3(model*Vec4(D.x,D.y,D.z,1));

}

void MeshQuad::decale_quad(int q, float d)
{
    Mat4 tr = translate(0,0,d);
    transfo_quad(q,tr);

    gl_update();
}

void MeshQuad::shrink_quad(int q, float s)
{
    Mat4 tr = scale(s);
    transfo_quad(q,tr);

    gl_update();
}

void MeshQuad::tourne_quad(int q, float a)
{
    Mat4 tr = rotateZ(a);
    transfo_quad(q,tr);

    gl_update();
}

MeshQuad::MeshQuad():
    m_nb_ind_edges(0)
{}


void MeshQuad::gl_init()
{
    m_shader_flat = new ShaderProgramFlat();
    m_shader_color = new ShaderProgramColor();

    //VBO
    glGenBuffers(1, &m_vbo);

    //VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(m_shader_flat->idOfVertexAttribute);
    glVertexAttribPointer(m_shader_flat->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &m_vao2);
    glBindVertexArray(m_vao2);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(m_shader_color->idOfVertexAttribute);
    glVertexAttribPointer(m_shader_color->idOfVertexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    //EBO indices
    glGenBuffers(1, &m_ebo);
    glGenBuffers(1, &m_ebo2);
}

void MeshQuad::gl_update()
{
    //VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * m_points.size() * sizeof(GLfloat), &(m_points[0][0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::vector<int> tri_indices;
    convert_quads_to_tris(m_quad_indices,tri_indices);

    //EBO indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,tri_indices.size() * sizeof(int), &(tri_indices[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::vector<int> edge_indices;
    convert_quads_to_edges(m_quad_indices,edge_indices);
    m_nb_ind_edges = edge_indices.size();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,m_nb_ind_edges * sizeof(int), &(edge_indices[0]), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



void MeshQuad::set_matrices(const Mat4& view, const Mat4& projection)
{
    viewMatrix = view;
    projectionMatrix = projection;
}

void MeshQuad::draw(const Vec3& color)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);

    m_shader_flat->startUseProgram();
    m_shader_flat->sendViewMatrix(viewMatrix);
    m_shader_flat->sendProjectionMatrix(projectionMatrix);
    glUniform3fv(m_shader_flat->idOfColorUniform, 1, glm::value_ptr(color));
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo);
    glDrawElements(GL_TRIANGLES, 3*m_quad_indices.size()/2,GL_UNSIGNED_INT,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    m_shader_flat->stopUseProgram();

    glDisable(GL_POLYGON_OFFSET_FILL);

    m_shader_color->startUseProgram();
    m_shader_color->sendViewMatrix(viewMatrix);
    m_shader_color->sendProjectionMatrix(projectionMatrix);
    glUniform3f(m_shader_color->idOfColorUniform, 0.0f,0.0f,0.0f);
    glBindVertexArray(m_vao2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo2);
    glDrawElements(GL_LINES, m_nb_ind_edges,GL_UNSIGNED_INT,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    m_shader_color->stopUseProgram();
    glDisable(GL_CULL_FACE);
}
