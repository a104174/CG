#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <filesystem>

#include <tinyxml2.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

using namespace std;
namespace fs = std::filesystem;

struct Vertex { float x, y, z; };

struct Camera {
    float pos[3]    = {0, 0, 5};
    float look[3]   = {0, 0, 0};
    float up[3]     = {0, 1, 0};
    float fov       = 60.0f;
    float nearPlane = 1.0f;
    float farPlane  = 1000.0f;
};

struct Model {
    vector<Vertex> verts; // triangulos (vértices “não indexados”)
};

static int gWidth = 800;
static int gHeight = 800;
static Camera gCam;
static vector<Model> gModels;

// toggles
static bool gWireframe = true;
static bool gShowAxes = true;

static bool load3D(const fs::path& filePath, Model& out) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Erro a abrir .3d: " << filePath << "\n";
        return false;
    }

    size_t n = 0;
    file >> n;
    out.verts.clear();
    out.verts.reserve(n);

    for (size_t i = 0; i < n; i++) {
        Vertex v{};
        file >> v.x >> v.y >> v.z;
        out.verts.push_back(v);
    }
    return true;
}

static bool readVec3(tinyxml2::XMLElement* elem, float v[3]) {
    if (!elem) return false;
    if (elem->QueryFloatAttribute("x", &v[0]) != tinyxml2::XML_SUCCESS) return false;
    if (elem->QueryFloatAttribute("y", &v[1]) != tinyxml2::XML_SUCCESS) return false;
    if (elem->QueryFloatAttribute("z", &v[2]) != tinyxml2::XML_SUCCESS) return false;
    return true;
}

static bool parseXML(const fs::path& xmlPath) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(xmlPath.string().c_str()) != tinyxml2::XML_SUCCESS) {
        cerr << "Erro a ler XML: " << xmlPath << "\n";
        return false;
    }

    auto* world = doc.FirstChildElement("world");
    if (!world) {
        cerr << "XML sem <world>\n";
        return false;
    }

    // window
    if (auto* win = world->FirstChildElement("window")) {
        win->QueryIntAttribute("width", &gWidth);
        win->QueryIntAttribute("height", &gHeight);
    }

    // camera
    if (auto* cam = world->FirstChildElement("camera")) {
        readVec3(cam->FirstChildElement("position"), gCam.pos);
        readVec3(cam->FirstChildElement("lookAt"), gCam.look);
        readVec3(cam->FirstChildElement("up"), gCam.up);

        if (auto* proj = cam->FirstChildElement("projection")) {
            proj->QueryFloatAttribute("fov", &gCam.fov);
            proj->QueryFloatAttribute("near", &gCam.nearPlane);
            proj->QueryFloatAttribute("far", &gCam.farPlane);
        }
    }

    // models (world/group/models/model)
    gModels.clear();

    for (auto* group = world->FirstChildElement("group"); group; group = group->NextSiblingElement("group")) {
        auto* models = group->FirstChildElement("models");
        if (!models) continue;

        for (auto* m = models->FirstChildElement("model"); m; m = m->NextSiblingElement("model")) {
            const char* fileAttr = m->Attribute("file");
            if (!fileAttr) continue;

            fs::path modelPath = xmlPath.parent_path() / fs::path(fileAttr);

            Model model;
            if (!load3D(modelPath, model)) {
                cerr << "Falhou carregar model: " << modelPath << "\n";
                return false;
            }
            gModels.push_back(std::move(model));
        }
    }

    if (gModels.empty()) {
        cerr << "Nenhum <model> encontrado (ou paths errados).\n";
        return false;
    }

    return true;
}

static void applyCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(gCam.fov, (double)gWidth / (double)gHeight, gCam.nearPlane, gCam.farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        gCam.pos[0],  gCam.pos[1],  gCam.pos[2],
        gCam.look[0], gCam.look[1], gCam.look[2],
        gCam.up[0],   gCam.up[1],   gCam.up[2]
    );
}

static void drawAxes(float len = 5.0f) {
    if (!gShowAxes) return;

    // Eixos em linhas (independente do wireframe dos modelos)
    glBegin(GL_LINES);

    // X (vermelho)
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(len, 0.f, 0.f);

    // Y (verde)
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, len, 0.f);

    // Z (azul)
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, len);

    glEnd();

    glColor3f(1.f, 1.f, 1.f);
}

static void renderModels() {
    // wireframe / solid
    glPolygonMode(GL_FRONT_AND_BACK, gWireframe ? GL_LINE : GL_FILL);

    // modelos em branco
    glColor3f(1.f, 1.f, 1.f);

    for (const auto& model : gModels) {
        glBegin(GL_TRIANGLES);
        for (const auto& v : model.verts) {
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
}

static void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    applyCamera();

    drawAxes();
    renderModels();

    glutSwapBuffers();
}

static void reshape(int w, int h) {
    if (h == 0) h = 1;
    gWidth = w; gHeight = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

static void keyboard(unsigned char key, int, int) {
    if (key == 27 || key == 'q' || key == 'Q') {
        exit(0);
    }

    if (key == 'w' || key == 'W') {
        gWireframe = !gWireframe;
        glutPostRedisplay();
    }

    if (key == 'a' || key == 'A') {
        gShowAxes = !gShowAxes;
        glutPostRedisplay();
    }
}

static void initGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.f, 0.f, 0.f, 1.f);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: engine <scene.xml>\n";
        return 1;
    }

    fs::path xmlPath = fs::path(argv[1]);

    if (!parseXML(xmlPath)) {
        return 1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(gWidth, gHeight);
    glutCreateWindow("CG Engine - Fase 1");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc([](){ glutPostRedisplay(); });

    glutMainLoop();
    return 0;
}