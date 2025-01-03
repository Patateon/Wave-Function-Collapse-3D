#include "tilemodel.h"

#include "BasicIO.h"
#include "OBJ_Loader.h"

//Ajout de paramtre de rotation possibles pour chaque direction au TileModel pour lorsque l'on set une instance du modele on pioche au hasard dedans.
//Ensuite quand on set quelque chose a coté il faudra récupérer la rotation du voisin le plus important(voir améliorer regles)
//Avec cette approche on estime que lorsque l'on charge un modele il est automatiquement de face lorsqu'on le rend sur la fenetre
TileModel::TileModel(uint id)
{
    m_id = id;
}

TileModel::TileModel(uint id, QString filename)
{
    m_id = id;
    setMesh(filename);
    QFileInfo fileInfo(filename);
    m_name = fileInfo.fileName();
    m_rotx=QVector<bool>(3,false);
    m_roty=QVector<bool>(3,false);
    m_rotz=QVector<bool>(3,false);
}

TileModel::TileModel(uint id, QSet<int> rules)
{
    m_id = id;
    m_rules=rules;

}

TileModel::~TileModel(){

}

void TileModel::loadOBJ(QString filename){
    objl::Loader loader;

    bool loaded = loader.LoadFile(filename.toStdString());

    if(!loaded){
        qWarning() << "Could not load obj : " << filename;
        return;
    }

    if(loader.LoadedMeshes.size() != 1) {
        qWarning() << "Unexpected number of meshes "
                   << "from a single file : "
                   << loader.LoadedMeshes.size();
    }

    objl::Mesh currentMesh = loader.LoadedMeshes[0];

    m_mesh.vertices.resize(currentMesh.Vertices.size());
    for(uint i = 0; i < currentMesh.Vertices.size(); i++){
        m_mesh.vertices[i] = Vertex(currentMesh.Vertices[i].Position.X,
                                    currentMesh.Vertices[i].Position.Y,
                                    currentMesh.Vertices[i].Position.Z);
    }

    m_mesh.normales.resize(currentMesh.Vertices.size());
    for(uint i = 0; i < currentMesh.Vertices.size(); i++){
        m_mesh.normales[i] = Vertex(currentMesh.Vertices[i].Normal.X,
                                    currentMesh.Vertices[i].Normal.Y,
                                    currentMesh.Vertices[i].Normal.Z);
    }

    for(uint i = 0; i < currentMesh.Indices.size(); i+=3){
        Triangle t;
        t[0] = currentMesh.Indices[i];
        t[1] = currentMesh.Indices[i+1];
        t[2] = currentMesh.Indices[i+2];
        m_mesh.triangles.push_back(t);
    }
}


void TileModel::setMesh(QString filename)
{
    if (filename.endsWith(".off")){
        OFFIO::openTriMesh(filename.toStdString(),
                                     m_mesh.vertices,
                                     m_mesh.triangles);
        m_mesh.computeNormales();
    }else if (filename.endsWith(".obj")){
        // Suppose la présence de normales
        loadOBJ(filename);
    }

    computeBoundingBox();
}

void TileModel::setRules(QSet<int> rules){
    m_rules=rules;
}

QSet<int> TileModel::getRules(){
    return m_rules;
}

void TileModel::setType(QVector<TileModel> modeles){
    if(m_rules.size()<=modeles.size()/3){
        m_type=1;
    }
    else{
        m_type=0;
    }
}

int TileModel::getType(){
    return m_type;
}

void TileModel::setRots(QVector<bool> rotx,QVector<bool> roty,QVector<bool> rotz){
    m_rotx=rotx;
    m_roty=roty;
    m_rotz=rotz;
}

bool TileModel::operator<(const TileModel & other) const{
    return m_id < other.m_id;
}

void TileModel::computeBoundingBox() {

    if (mesh().vertices.size() <= 0){
        m_bbmin = QVector3D(0.0f, 0.0f, 0.0f);
        m_bbmax = QVector3D(0.0f, 0.0f, 0.0f);
        return;
    }

    m_bbmin = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
    m_bbmax = QVector3D(FLT_MIN, FLT_MIN, FLT_MIN);

    for(uint i = 0; i < mesh().vertices.size(); i++){
        for(uint k = 0; k < 3; k++){
            if (mesh().vertices[i][k] > m_bbmax[k]){
                m_bbmax[k] = mesh().vertices[i][k];
            }
            if (mesh().vertices[i][k] < m_bbmin[k]){
                m_bbmin[k] = mesh().vertices[i][k];
            }
        }
    }
}

QVector<bool> TileModel::getXRot(){
    return m_rotx;
}

QVector<bool> TileModel::getYRot(){
    return m_roty;
}

QVector<bool> TileModel::getZRot(){
    return m_rotz;
}

QString TileModel::getName(){
    return m_name;
}

