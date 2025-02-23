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

TileModel::TileModel(uint id, QVector<QSet<int>> rules)
{
    m_id = id;
  
    // Rules
    m_rules_xminus=rules[0];
    m_rules_xplus=rules[1];
    m_rules_yminus=rules[2];
    m_rules_yplus=rules[3];
    m_rules_zminus=rules[4];
    m_rules_zplus=rules[5];

}

TileModel::~TileModel(){
    // delete m_mesh;
}

void TileModel::loadOBJ(QString filename){
    Mesh *mesh = new Mesh;
    objl::Loader loader;

    bool loaded = loader.LoadFile(filename.toStdString());

    if(!loaded){
        mesh->vertices.clear();
        mesh->triangles.clear();
        mesh->normales.clear();
        m_mesh = mesh;
        qWarning() << "Could not load obj : " << filename;
        qWarning() << "Loaded as an empty mesh";
        return;
    }

    if(loader.LoadedMeshes.size() != 1) {
        qWarning() << "Unexpected number of meshes "
                   << "from a single file : "
                   << loader.LoadedMeshes.size();
    }

    objl::Mesh currentMesh = loader.LoadedMeshes[0];

    mesh->vertices.resize(currentMesh.Vertices.size());
    for(uint i = 0; i < currentMesh.Vertices.size(); i++){
        mesh->vertices[i] = Vertex(currentMesh.Vertices[i].Position.X,
                                    currentMesh.Vertices[i].Position.Y,
                                    currentMesh.Vertices[i].Position.Z);
    }

    mesh->normales.resize(currentMesh.Vertices.size());
    for(uint i = 0; i < currentMesh.Vertices.size(); i++){
        mesh->normales[i] = Vertex(currentMesh.Vertices[i].Normal.X,
                                    currentMesh.Vertices[i].Normal.Y,
                                    currentMesh.Vertices[i].Normal.Z);
    }

    for(uint i = 0; i < currentMesh.Indices.size(); i+=3){
        Triangle t;
        t[0] = currentMesh.Indices[i];
        t[1] = currentMesh.Indices[i+1];
        t[2] = currentMesh.Indices[i+2];
        mesh->triangles.push_back(t);
    }

    m_mesh = mesh;
}


void TileModel::setMesh(QString filename)
{
    if (filename.endsWith(".off")){
        Mesh * mesh = new Mesh;
        OFFIO::openTriMesh(filename.toStdString(),
                                     mesh->vertices,
                                     mesh->triangles);
        mesh->computeNormales();
        m_mesh = mesh;
    }else if (filename.endsWith(".obj")){
        // Suppose la présence de normales
        loadOBJ(filename);
    }else {
        return;
    }

    computeBoundingBox();
    centerMesh();
    computeBoundingBox();
}

void TileModel::setRules(QSet<int> rules_xminus,QSet<int> rules_xplus,QSet<int> rules_yminus,QSet<int> rules_yplus,QSet<int> rules_zminus,QSet<int> rules_zplus){
    m_rules_xminus=rules_xminus;
    m_rules_xplus=rules_xplus;
    m_rules_yminus=rules_yminus;
    m_rules_yplus=rules_yplus;
    m_rules_zminus=rules_zminus;
    m_rules_zplus=rules_zplus;
}

QVector<QSet<int>*> TileModel::getRules() {
    QVector<QSet<int>*> rules;
    rules.push_back(&m_rules_xminus);
    rules.push_back(&m_rules_xplus);
    rules.push_back(&m_rules_yminus);
    rules.push_back(&m_rules_yplus);
    rules.push_back(&m_rules_zminus);
    rules.push_back(&m_rules_zplus);
    return rules;
}

QSet<int> TileModel::getXMinus(){
    return m_rules_xminus;
}

QSet<int> TileModel::getXPlus(){
    return m_rules_xplus;
}

QSet<int> TileModel::getYMinus(){
    return m_rules_yminus;
}

QSet<int> TileModel::getYPlus(){
    return m_rules_yplus;
}

QSet<int> TileModel::getZMinus(){
    return m_rules_zminus;
}

QSet<int> TileModel::getZPlus(){
    return m_rules_zplus;
}

void TileModel::setType(QVector<TileModel*> &modeles,int mode){
    if(mode==0){
        float meanN=(m_rules_xminus.size()+m_rules_xplus.size()+m_rules_yminus.size()+m_rules_yplus.size()+m_rules_zminus.size()+m_rules_zplus.size())/6.0f;
        if(meanN<=modeles.size()/2){
            m_type=1;
        }
        else{
            m_type=0;
        }
    }
    else{
        if(m_rules_xminus.size()/2){
            m_type=1;
        }
        else{
            m_type=0;
        }
    }
}

int TileModel::getType() const{
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

    if (m_mesh == nullptr){
        return;
    }

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

void TileModel::centerMesh(){

    // Appellé uniquement après computeBoudingBox

    if (m_mesh == nullptr){
        qWarning() << "Could not normalize mesh";
        return;
    }

    QVector3D center = (m_bbmax + m_bbmin) / 2.0f;

    // if (size.x() == 0 && size.y() == 0 && size.z() == 0){
    //     qWarning() << "Could not normalize mesh";
    //     return;
    // }

    // for (uint i = 0; i < m_mesh->vertices.size(); i++){

    //     mesh().vertices[i].p[0] = ((2.0f * mesh().vertices[i].p.x() - m_bbmin.x())
    //                                / size.x()) - 1.0f;
    //     mesh().vertices[i].p[1] = ((2.0f * mesh().vertices[i].p.y() - m_bbmin.y())
    //                                / size.y()) - 1.0f;
    //     mesh().vertices[i].p[2] = ((2.0f * mesh().vertices[i].p.z() - m_bbmin.z())
    //                                / size.z()) - 1.0f;
    // }

    // QVector3D barycentre = QVector3D();

    // for (uint i =0 ; i < mesh().vertices.size(); i++){
    //     point3d p = mesh().vertices[i].p;
    //     barycentre += QVector3D(p.x(), p.y(), p.z());
    // }

    // barycentre /= mesh().vertices.size();

    for (uint i = 0; i < mesh().vertices.size(); i++){
        point3d c = point3d(center.x(), center.y(), center.z());
        mesh().vertices[i].p -= c;
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

