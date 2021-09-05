#ifndef GEM_ROOT_CLUSTER_TREE_H
#define GEM_ROOT_CLUSTER_TREE_H

#include <TTree.h>
#include <TFile.h>

class GEMSystem;
class GEMCluster;

////////////////////////////////////////////////////////////////////////////////
// replay evio files, and cluster all hits, save clusters to root tree

#define MAXCLUSTERS 200000
#define MAXCLUSTERSIZE 100

class GEMRootClusterTree
{
public:
    GEMRootClusterTree(const char *path);
    ~GEMRootClusterTree();

    void Write();
    void Fill(GEMSystem* gem_sys, const uint32_t &evt_num);

private:
    TTree *pTree = nullptr;
    TFile *pFile = nullptr;

    std::string fPath;

    // information to save
    int evtID;
    int nCluster;
    int Plane[MAXCLUSTERS];
    int Prod[MAXCLUSTERS]; 
    int Module[MAXCLUSTERS];
    int Axis[MAXCLUSTERS];
    int Size[MAXCLUSTERS];

    float Adc[MAXCLUSTERS];
    float Pos[MAXCLUSTERS];

    int StripNo[MAXCLUSTERS][MAXCLUSTERSIZE];   // max 50 strips per cluster
    float StripADC[MAXCLUSTERS][MAXCLUSTERSIZE];

    // clustering method
    GEMCluster *cluster_method = nullptr;
};

#endif
