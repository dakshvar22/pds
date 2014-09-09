#ifndef PDS_H
#define PDS_H

// Error codes
#define PDS_SUCCESS 0
#define PDS_FILE_ERROR 3
#define PDS_ADD_FAILED 2
#define PDS_REC_NOT_FOUND 1
#define PDS_REPO_ALREADY_OPEN 12
#define PDS_NDX_SAVE_FAILED 13
#define PDS_NDX_BLOCK_FULL 14

// Number of index entries in each block
#define BLOCK_SIZE 5

// Maximum number of index blocks in index file
#define MAX_BLOCKS 6

// Repository status values
#define PDS_REPO_OPEN 10
#define PDS_REPO_CLOSED 11
#define PDS_REPO_CREATE 20
// Constants for key bins
#define KEY_MIN 1000
#define KEY_BIN_SIZE 1000
#define NULL_KEY -1

// Each index block will contain keys in the range KEY_MIN <= key < KEY_MIN + KEY_BIN_SIZE

struct PDS_NdxInfo{
	int key;
	int offset;
};

struct PDS_NdxBlock{
	// The following two fields constitute the block header
	// ndx_block_num == 0 contains level-2 index
	int ndx_block_num;
	// Number of entries that are used
	int used_size;
	// Sorted NdxInfo entries
	struct PDS_NdxInfo ndx[BLOCK_SIZE];
};	

struct PDS_RepoInfo{
	char pds_name[30];
	FILE *pds_data_fp;
	FILE *pds_ndx_fp;
	int repo_status;
	int repo_create_status; 
	int rec_size; // For fixed length records
	struct PDS_NdxBlock ndx_block0;
};

extern struct PDS_RepoInfo repo_handle;

int pds_open( char *repo_name, int rec_size );

// Read index block into memory
int pds_load_primary_ndx();

int put_rec_by_key( int key, void *rec );
int get_rec_by_key( int key, void *rec );
int pds_close();

#endif
