#include "GEMRootClusterTree.h"
#include "GEMStruct.h"
#include "GEMSystem.h"
#include "GEMCluster.h"
#include "APVStripMapping.h"

#include <iostream>

GEMRootClusterTree::GEMRootClusterTree(const char* path)
{
    fPath = path;
    pFile = new TFile(path, "RECREATE");
    pTree = new TTree("GEMCluster", "cluster list");

    pTree -> Branch("evtID", &evtID, "evtID/I");
    pTree -> Branch("nCluster", &nCluster, "nCluster/I");
    pTree -> Branch("planeID", Plane, "planeID[nCluster]/I");
    pTree -> Branch("prodID", Prod, "prodID[nCluster]/I");
    pTree -> Branch("moduleID", Module, "moduleID[nCluster]/I");
    pTree -> Branch("axis", Axis, "axis[nCluster]/I");
    pTree -> Branch("size", Size, "size[nCluster]/I");
    pTree -> Branch("adc", Adc, "adc[nCluster]/F");
    pTree -> Branch("pos", Pos, "Pos[nCluster]/F");

    // save strip information for each cluster
    pTree -> Branch("stripNo", StripNo, "StripNo[nCluster][100]/I");
    pTree -> Branch("stripAdc", StripADC, "StripADC[nCluster][100]/F");
}

GEMRootClusterTree::~GEMRootClusterTree()
{
    // place holder
}

void GEMRootClusterTree::Write()
{
    std::cout<<"Writing root cluster tree to : "<<fPath<<std::endl;
    pFile -> Write();
    pFile -> Close();
}

// a helper to get chamber based strip index, to be removed

static int getChamberBasedStripNo(int strip, int type, int N_APVS_PER_PLANE, int detLayerPositionIndex)
{
    // no conversion for Y plane
    if(type == 1)
        return strip;

    // conversion for X plane
    int c_strip = strip - N_APVS_PER_PLANE * 128 * detLayerPositionIndex;
    if(strip < 0)
    {
        std::cout<<"Error: strip conversion failed, returned without conversion."
                 <<std::endl;
        return strip;
    }
    return c_strip;
}

void GEMRootClusterTree::Fill(GEMSystem *gem_sys, const uint32_t &evt_num)
{
    if(cluster_method == nullptr)
        cluster_method = new GEMCluster("config/gem_cluster.conf");

    [[maybe_unused]]int ndet = apv_strip_mapping::Mapping::Instance()->GetTotalNumberOfDetectors();

    // set event id
    evtID = static_cast<int>(evt_num);
    nCluster = 0;

    // get detector list
    std::vector<GEMDetector*> detectors = gem_sys -> GetDetectorList();
    for(auto &i: detectors) 
    {
        std::vector<GEMPlane*> planes = i->GetPlaneList();

        for(auto &pln: planes) 
        {
            const std::vector<StripCluster> & clusters = pln -> GetStripClusters();
            int napvs_per_plane = pln -> GetCapacity();
            for(auto &c: clusters) {
                Plane[nCluster] = i -> GetLayerID();
                Prod[nCluster] = i -> GetDetID();
                Module[nCluster] = i -> GetDetLayerPositionIndex();
                Axis[nCluster] = static_cast<int>(pln -> GetType());
                Size[nCluster] = c.hits.size();
                Adc[nCluster] = c.peak_charge;
                Pos[nCluster] = c.position;

                // strips in this cluster
                const std::vector<StripHit> &hits = c.hits;
                for(size_t nS = 0; nS < hits.size() && nS < 100; ++nS)
                {
                    // layer based strip no
                    //StripNo[nCluster][nS] = hits[nS].strip;

                    // chamber based strip no
                    StripNo[nCluster][nS] = getChamberBasedStripNo(hits[nS].strip, Axis[nCluster],
                           napvs_per_plane, Module[nCluster]);
 
                    StripADC[nCluster][nS] = hits[nS].charge;
                }

                nCluster++;
                if(nCluster >= MAXCLUSTERS)
                    break;
            }
        }
    }

    if(nCluster > 0)
        pTree -> Fill();
}
