//
//  banded_global_aligner.hpp
//  
//
//  Created by Jordan Eizenga on 7/25/16.
//
//

#ifndef banded_global_aligner_hpp
#define banded_global_aligner_hpp

#include <stdio.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include "vg.pb.h"

#endif /* banded_global_aligner_hpp */

using namespace std;

namespace vg {
    class BandedAlignmentMatrix {
    private:
        
        enum matrix {Match, InsertCol, InsertRow};
        
        // these indicate the diagonals in this matrix that the band passes through
        // the bottom index is inclusive
        int64_t top_diag;
        int64_t bottom_diag;
        
        Node* node;
        
        const char* read;
        int64_t read_seq_len;
        
        int64_t cumulative_seq_len;
        
        BandedAlignmentMatrix** seeds;
        int64_t num_seeds;
        
        int8_t* match;
        int8_t* insert_col;
        int8_t* insert_row;
        
        void traceback_internal(stringstream& strm, int64_t start_row, int64_t start_col, matrix start_mat,
                                bool in_lead_gap, int8_t* score_mat, int8_t* nt_table, int8_t gap_open,
                                int8_t gap_extend);
        
    public:
        BandedAlignmentMatrix(string& read, Node* node, int64_t top_diag, int64_t bottom_diag,
                              BandedAlignmentMatrix** seeds, int64_t num_seeds, int64_t cumulative_seq_len);
        BandedAlignmentMatrix();
        ~BandedAlignmentMatrix();
        
        bool is_masked();
        
        void fill_matrix(int8_t* score_mat, int8_t* nt_table, int8_t gap_open, int8_t gap_extend);
        
        int8_t final_score();
        
        // TODO: coordinate with Erik about traceback semantics
        void traceback(stringstream& strm, int8_t* score_mat, int8_t* nt_table, int8_t gap_open, int8_t gap_extend);
    };
    
    class BandedGlobalAlignmentGraph {
    public:
        
        BandedGlobalAlignmentGraph(string& read, Graph& g, int64_t band_width);
        ~BandedGlobalAlignmentGraph();
        
        // perform dynamic programming through the band
        void align(int8_t* score_mat, int8_t* nt_table, int8_t gap_open, int8_t gap_extend);
        // get traceback string after aligning
        string traceback(int8_t* score_mat, int8_t* nt_table, int8_t gap_open, int8_t gap_extend);
        
    private:
        int64_t band_width;
        vector<BandedAlignmentMatrix> banded_matrices;
        vector<Node*> topological_order;
        unordered_set<Node*> source_nodes;
        unordered_set<Node*> sink_nodes;
        
        // construction functions
        void topological_sort(Graph& g, vector<vector<int64_t>>& node_edges_out, vector<Node*>& out_topological_order);
        void graph_edge_lists(Graph& g, bool outgoing_edges, vector<vector<int64_t>>& edge_list);
        void find_banded_paths(string& read, vector<vector<int64_t>>& node_edges_in, int64_t band_width,
                               vector<Node*>& topological_order, vector<bool>& node_masked,
                               vector<pair<int64_t, int64_t>>& band_ends);
        void shortest_seq_paths(vector<Node*>& topological_order, vector<vector<int64_t>>& node_edges_out,
                                vector<int64_t>& seq_lens_out);
    };
}


