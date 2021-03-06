
#ifndef VG_TRANSCRIPTOME_HPP_INCLUDED
#define VG_TRANSCRIPTOME_HPP_INCLUDED

#include <algorithm>
#include <mutex>

#include <google/protobuf/util/message_differencer.h>
#include <gbwt/dynamic_gbwt.h>

#include "../vg.hpp"
#include "../path_index.hpp"
#include "../types.hpp"

namespace vg {

using namespace std;


typedef vector<gbwt::node_type> exon_nodes_t;


/**
 * Data structure that defines a transcript.
 */
struct Transcript {

    /// Transcript name.
    const string name;

    /// Is transcript in reverse direction (strand == '-').
    const bool is_reverse;

    /// Name of chromosome/contig where transcript exist.
    const string chrom;
    
    /// Exon coordinates (start and end) on the chromosome/contig.
    vector<pair<int32_t, int32_t> > exons;

    /// Exon positions (start and end) on a variation graph. 
    vector<pair<Position, Position> > exon_nodes;

    Transcript(const string & name_in, const bool is_reverse_in, const string & chrom_in) : name(name_in), is_reverse(is_reverse_in), chrom(chrom_in) {}
};


/**
 * Data structure that defines a transcript path.
 */ 
struct TranscriptPath {

    /// Transcript path.
    Path path;

    /// Total number of copies.
    int32_t num_total;

    /// Number of reference chromosome/contig copies.           
    int32_t num_reference;

    TranscriptPath(const bool is_reference) : num_total(1), num_reference(is_reference) {}
};


/**
 * Class that defines a transcriptome represented by a set of paths.
 */
class Transcriptome {

    public:

        Transcriptome(const string &, const bool);   
        ~Transcriptome();

        /// Number of threads used for transcript path construction. 
        int32_t num_threads = 1;

        /// Attribute tag used to parse the transcript id/name in the gtf/gff file. 
        string transcript_tag = "transcript_id";

        /// Use paths embedded in the graph for transcript path construction. 
        bool use_embedded_paths = false;

        /// Collapse identical transcript paths.
        bool collapse_transcript_paths = true;

        /// Filter transcript paths originating from a reference chromosome/contig.
        bool filter_reference_transcript_paths = false;

        /// Constructs transcript paths by projecting transcripts from a gtf/gff file onto 
        /// embedded paths in a variation graph and/or haplotypes in a GBWT index.   
        void add_transcripts(istream & transcript_stream, const gbwt::GBWT & haplotype_index);
        
        /// Returns transcript paths.
        const vector<Path> & transcript_paths() const;

        /// Returns number of transcript paths.
        int32_t size() const;

        /// Returns spliced variation graph.
        const VG & splice_graph() const; 

        /// Embeds transcript paths in variation graph.
        void add_paths_to_graph();

        /// Removes non-transcribed (not in transcript paths) nodes
        void remove_non_transcribed();

        /// Add transcript paths as threads in GBWT index.
        void construct_gbwt(gbwt::GBWTBuilder * gbwt_builder) const;
        
        /// Writes transcript paths as alignments to a gam file.
        void write_gam_alignments(ostream * gam_ostream) const;

        /// Writes transcript path sequences to a fasta file.  
        void write_fasta_sequences(ostream * fasta_ostream);

        /// Writes spliced variation graph to vg file
        void write_splice_graph(ostream * graph_ostream);

    private:

        /// Transcriptome represented by a set of transcript paths. 
        /// TODO: Change to vector<TranscriptPath>. Current implementation
        ///       decided in order for it to work easily with edit without 
        ///       excessive copying and moving of paths. 
        vector<Path> _transcriptome;

        /// Mutex used for adding transcript paths to transcriptome
        mutex trancriptome_mutex;

        /// Spliced variation graph.
        VG * graph;

        /// Finds the position of each end of a exon on a path in the  
        /// variation graph and adds the exon to a transcript.
        void add_exon(Transcript * transcript, const pair<int32_t, int32_t> & exon_pos, const PathIndex & chrom_path_index) const;

        /// Reverses exon order if the transcript is on the reverse strand and the exons 
        /// are ordered in reverse.
        void reorder_exons(Transcript * transcript) const;

        /// Constructs transcript paths by projecting transcripts onto embedded paths 
        /// in a variation graph and/or haplotypes in a GBWT index.
        void project_transcripts(const vector<Transcript> & transcripts, const gbwt::GBWT & haplotype_index, const float mean_node_length);

        /// Threaded transcript projecting.
        void project_transcripts_callback(const int32_t thread_idx, const vector<Transcript> & transcripts, const gbwt::GBWT & haplotype_index, const float mean_node_length);

        /// Projects transcripts onto haplotypes in a GBWT index and returns resulting transcript paths.
        list<TranscriptPath> project_transcript_gbwt(const Transcript & cur_transcript, const gbwt::GBWT & haplotype_index, const float mean_node_length) const;

        /// Extracts all unique haplotype paths between two nodes from a GBWT index and returns the 
        /// resulting paths and the corresponding haplotype ids for each path.
        vector<pair<exon_nodes_t, vector<gbwt::size_type> > > get_exon_haplotypes(const vg::id_t start_node, const vg::id_t end_node, const gbwt::GBWT & haplotype_index, const int32_t expected_length) const;

        /// Projects transcripts onto embedded paths in a variation graph and returns resulting transcript paths.
        list<TranscriptPath> project_transcript_embedded(const Transcript & cur_transcript);

        /// Collapses identical transcript paths. The number of collapsed copies are 
        /// recorded in the data structure TranscriptPath.
        void collapse_identical_paths(list<TranscriptPath> * cur_transcript_paths) const;

        /// Edits variation graph with transcript path splice-junctions and 
        /// updates transcript path traversals to match the augmented graph. 
        void add_junctions_to_graph();        
};

}

#endif
