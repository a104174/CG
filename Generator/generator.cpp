//
// Created by joaoveloso on 13/02/26.
// Edited by heldercruz on 22/02/26.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

// static_cast<novo_tipo>(valor)  -> converte o valor num novo tipo

using namespace std;

struct Vertex {
    float x, y, z;
};

static void write3D(const string& filename, const vector<Vertex>& vertices) {
    // Criar ficheiro de output
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error creating file\n";
        return;
    }

    file << vertices.size() << "\n";
    for (const auto& v : vertices) {
        file << v.x << " " << v.y << " " << v.z << "\n";
    }
    file.close();
}


static void generatePlane (float length, int divisions, vector<Vertex>& vertices) {
    float half = length / 2.0f;
    float step = length / static_cast<float>(divisions);

    // i->Z (linhas), j->X (colunas)
    for (int i = 0; i < divisions; i++) {
        float z0 = -half + i * step;
        float z1 = z0 + step;

        for (int j = 0; j < divisions; j++) {
            float x0 = -half + j * step;
            float x1 = x0 + step;

            // quadrado: (x0, z0) (x1,z0) (x1,z1) (x0, z1) no plano y=0
            // p00(x0,0,z0) p10(x1,0,z0) p01(x0,0,z1) p11(x1,0,z1)

            // p00 ---- p10
            // |    \    |
            // |     \   |
            // p01 ---- p11

            // triângulo1: p00, p11, p10
            vertices.push_back({x0, 0.0f, z0});
            vertices.push_back({x1, 0.0f, z1});
            vertices.push_back({x1, 0.0f, z0});

            // triângulo2: p00, p01, p11
            vertices.push_back({x0, 0.0f, z0});
            vertices.push_back({x0, 0.0f, z1});
            vertices.push_back({x1, 0.0f, z1});
        }
    }
}

// length define os limites [-half, half] em X, Y e Z
// grid define em quantos quadrados cada face é dividida (ex: grid=2 -> 4 quadrados / 8 triângulos por face, grid=3 -> 9 quadrados / 18 triângulos por face, etc)
static void generateBox (float length, int grid, vector<Vertex>& vertices) {
    float half = length / 2.0f;
    float step = length / static_cast<float>(grid);

    auto addQuad = [&](const Vertex& q00, const Vertex& q10, const Vertex& q01, const Vertex& q11, bool flip) {
        // q00 --- q10
        // |      |
        // q01 --- q11

        if (!flip) {
            // tri1: q00, q01, q11
            vertices.push_back(q00);
            vertices.push_back(q01);
            vertices.push_back(q11);

            // tri2: q00, q11, q10
            vertices.push_back(q00);
            vertices.push_back(q11);
            vertices.push_back(q10);

        } else { // invert winding
            // tri1: q00, q11, q01
            vertices.push_back(q00);
            vertices.push_back(q11);
            vertices.push_back(q01);

            // tri2: q00, q10, q11
            vertices.push_back(q00);
            vertices.push_back(q10);
            vertices.push_back(q11);
        }
    };

    // Generate box faces
    // X = +-half, Y/Z vary
    for (int i = 0; i < grid; i++) {
        float z0 = -half + i * step;
        float z1 = z0 + step;

        for (int j = 0; j < grid; j++) {
            float y0 = -half + j * step;
            float y1 = y0 + step;

            Vertex x00f = {half, y0, z0};
            Vertex x10f = {half, y1, z0};
            Vertex x01f = {half, y0, z1};
            Vertex x11f = {half, y1, z1};

            Vertex x00b = {-half, y0, z0};
            Vertex x10b = {-half, y1, z0};
            Vertex x01b = {-half, y0, z1};
            Vertex x11b = {-half, y1, z1};

            addQuad(x00f, x10f, x01f, x11f, false);
            addQuad(x00b, x10b, x01b, x11b, true);
        }
    }

    // Y = +-half, X/Z vary
    for (int i = 0; i < grid; i++) {
        float z0 = -half + i * step;
        float z1 = z0 + step;

        for (int j = 0; j < grid; j++) {
            float x0 = -half + j * step;
            float x1 = x0 + step;

            Vertex y00f = {x0, half, z0};
            Vertex y10f = {x1, half, z0};
            Vertex y01f = {x0, half, z1};
            Vertex y11f = {x1, half, z1};

            Vertex y00b = {x0, -half, z0};
            Vertex y10b = {x1, -half, z0};
            Vertex y01b = {x0, -half, z1};
            Vertex y11b = {x1, -half, z1};

            addQuad(y00f, y10f, y01f, y11f, false);
            addQuad(y00b, y10b, y01b, y11b, true);
        }
    }

    // Z = +-half, X/Y vary
    for (int i = 0; i < grid; i++) {
        float y0 = -half + i * step;
        float y1 = y0 + step;

        for (int j = 0; j < grid; j++) {
            float x0 = -half + j * step;
            float x1 = x0 + step;

            Vertex z00f = {x0, y0, half};
            Vertex z10f = {x1, y0, half};
            Vertex z01f = {x0, y1, half};
            Vertex z11f = {x1, y1, half};

            Vertex z00b = {x0, y0, -half};
            Vertex z10b = {x1, y0, -half};
            Vertex z01b = {x0, y1, -half};
            Vertex z11b = {x1, y1, -half};

            addQuad(z00f, z10f, z01f, z11f, false);
            addQuad(z00b, z10b, z01b, z11b, true);
        }
    }
}

// Sphere: radius, slices, stacks
static void generateSphere(float radius, int slices, int stacks, vector<Vertex>& vertices) {
    const float PI = 3.14159265358979323846f;

    float dPhi = PI / static_cast<float>(stacks);
    float dTheta = 2.0f * PI / static_cast<float>(slices);

    for (int i = 0; i < stacks; i++) {
        float phi0 = -PI / 2.0f + i * dPhi;
        float phi1 = phi0 + dPhi;

        float y0 = radius * sinf(phi0);
        float y1 = radius * sinf(phi1);

        float r0 = radius * cosf(phi0);
        float r1 = radius * cosf(phi1);

        for (int j = 0; j < slices; j++) {
            float theta0 = j * dTheta;
            float theta1 = theta0 + dTheta;

            Vertex p00 = {r0 * sinf(theta0), y0, r0 * cosf(theta0)};
            Vertex p10 = {r0 * sinf(theta1), y0, r0 * cosf(theta1)};
            Vertex p01 = {r1 * sinf(theta0), y1, r1 * cosf(theta0)};
            Vertex p11 = {r1 * sinf(theta1), y1, r1 * cosf(theta1)};

            // 2 triângulos por célula
            // tri1: p00, p01, p11
            vertices.push_back(p00);
            vertices.push_back(p01);
            vertices.push_back(p11);

            // tri2: p00, p11, p10
            vertices.push_back(p00);
            vertices.push_back(p11);
            vertices.push_back(p10);
        }
    }
}

// Cone: radius, height, slices, stacks
static void generateCone(float radius, float height, int slices, int stacks, vector<Vertex>& vertices) {
    const float PI = 3.14159265358979323846f;

    float dTheta = 2.0f * PI / static_cast<float>(slices);
    float dH = height / static_cast<float>(stacks);

    // Base (y=0) - triangle fan
    Vertex center = {0.0f, 0.0f, 0.0f};
    for (int j = 0; j < slices; j++) {
        float theta0 = j * dTheta;
        float theta1 = theta0 + dTheta;

        Vertex p0 = {radius * sinf(theta0), 0.0f, radius * cosf(theta0)};
        Vertex p1 = {radius * sinf(theta1), 0.0f, radius * cosf(theta1)};

        // orientação da base (não é crucial na fase 1 sem culling)
        vertices.push_back(center);
        vertices.push_back(p1);
        vertices.push_back(p0);
    }

    // Lateral (stacks)
    for (int i = 0; i < stacks; i++) {
        float y0 = i * dH;
        float y1 = (i + 1) * dH;

        float r0 = radius * (1.0f - static_cast<float>(i) / static_cast<float>(stacks));
        float r1 = radius * (1.0f - static_cast<float>(i + 1) / static_cast<float>(stacks));

        for (int j = 0; j < slices; j++) {
            float theta0 = j * dTheta;
            float theta1 = theta0 + dTheta;

            Vertex p00 = {r0 * sinf(theta0), y0, r0 * cosf(theta0)};
            Vertex p10 = {r0 * sinf(theta1), y0, r0 * cosf(theta1)};
            Vertex p01 = {r1 * sinf(theta0), y1, r1 * cosf(theta0)};
            Vertex p11 = {r1 * sinf(theta1), y1, r1 * cosf(theta1)};

            // 2 triângulos por célula
            // tri1: p00, p01, p11
            vertices.push_back(p00);
            vertices.push_back(p01);
            vertices.push_back(p11);

            // tri2: p00, p11, p10
            vertices.push_back(p00);
            vertices.push_back(p11);
            vertices.push_back(p10);
        }
    }
}

int main(int argc, char** argv) {

    // Verificar argumentos
    if (argc < 2) {
        cout << "Usage: generator <primitive> ... <output_file>\n";
        return 1;
    }

    // Ler argumentos
    string primitive = argv[1];

    if (primitive == "plane") {

        if (argc != 5) {
            cout << "Usage: generator plane <length> <divisions> <output_file>\n";
            return 1;
        }

        float length = stof(argv[2]);
        int divisions = stoi(argv[3]);
        string output_filename = argv[4];

        if (length <= 0.0f || divisions < 1) {
            cerr << "Error: length > 0 and divisions >= 1\n";
            return 1;
        }

        vector<Vertex> vertices;
        // 2 triangulos por celula
        size_t nVertices = static_cast<size_t>(6 * divisions * divisions);
        vertices.reserve(nVertices);

        generatePlane(length, divisions, vertices);
        write3D(output_filename, vertices);

        cout << "Wrote " << output_filename << " (" << vertices.size() << " vertices)\n";
        cout << "File created sucessfully: " << output_filename << "\n";
        return 0;
    }

    if (primitive == "box") {

        if (argc != 5) {
            cout << "Usage: generator box <length> <grid> <output_file>\n";
            return 1;
        }

        float length = stof(argv[2]);
        int grid = stoi(argv[3]);
        string output_filename = argv[4];

        if (length <= 0.0f || grid < 1) {
            cerr << "Error: length > 0 and grid >= 1\n";
            return 1;
        }

        vector<Vertex> vertices;
        // 6 faces, grid*grid quadrados por face, 2 triangulos por quadrado, 3 vertices por triangulo (6 vertices por quadrado)
        size_t nVertices = static_cast<size_t>(36 * grid * grid);
        vertices.reserve(nVertices);

        generateBox(length, grid, vertices);
        write3D(output_filename, vertices);

        cout << "Wrote " << output_filename << " (" << vertices.size() << " vertices)\n";
        cout << "File created sucessfully: " << output_filename << "\n";
        return 0;
    }

    if (primitive == "sphere") {

        if (argc != 6) {
            cout << "Usage: generator sphere <radius> <slices> <stacks> <output_file>\n";
            return 1;
        }

        float radius = stof(argv[2]);
        int slices = stoi(argv[3]);
        int stacks = stoi(argv[4]);
        string output_filename = argv[5];

        if (radius <= 0.0f || slices < 3 || stacks < 2) {
            cerr << "Error: radius > 0, slices >= 3 and stacks >= 2\n";
            return 1;
        }

        vector<Vertex> vertices;
        // 2 triângulos por célula, slices*stacks células
        size_t nVertices = static_cast<size_t>(6 * slices * stacks);
        vertices.reserve(nVertices);

        generateSphere(radius, slices, stacks, vertices);
        write3D(output_filename, vertices);

        cout << "Wrote " << output_filename << " (" << vertices.size() << " vertices)\n";
        cout << "File created sucessfully: " << output_filename << "\n";
        return 0;
    }

    if (primitive == "cone") {

        if (argc != 7) {
            cout << "Usage: generator cone <radius> <height> <slices> <stacks> <output_file>\n";
            return 1;
        }

        float radius = stof(argv[2]);
        float height = stof(argv[3]);
        int slices = stoi(argv[4]);
        int stacks = stoi(argv[5]);
        string output_filename = argv[6];

        if (radius <= 0.0f || height <= 0.0f || slices < 3 || stacks < 1) {
            cerr << "Error: radius > 0, height > 0, slices >= 3 and stacks >= 1\n";
            return 1;
        }

        vector<Vertex> vertices;
        // base: slices triângulos => 3*slices vértices
        // lateral: stacks*slices células => 2 triângulos por célula => 6*slices*stacks vértices
        size_t nVertices = static_cast<size_t>(3 * slices + 6 * slices * stacks);
        vertices.reserve(nVertices);

        generateCone(radius, height, slices, stacks, vertices);
        write3D(output_filename, vertices);

        cout << "Wrote " << output_filename << " (" << vertices.size() << " vertices)\n";
        cout << "File created sucessfully: " << output_filename << "\n";
        return 0;
    }

    cout << "Unknown primitive: " << primitive << "\n";
    return 1;
}