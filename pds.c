#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<stdlib.h>

//#include "bst.h"
#include "pds.h"

// Define the global variable
struct PDS_RepoInfo repo_handle;
static void print();

int search_ndx(int search_key, struct PDS_NdxBlock *blk,int beg,int end)
{
		int mid = (beg+end)/2;
		while(beg<=end)
		{
			mid = (beg+end)/2;
			if(blk->ndx[mid].key == search_key)
			{
				return mid;
			}
			else if(search_key > blk->ndx[mid].key)
			{
				beg = mid+1;
			}
			else if(search_key < blk->ndx[mid].key)
			{
				end = mid-1;
			}
		}
		return -1;
}

int pds_create( char *repo_name )
{
	char repo_file[30];
	char ndx_file[30];

	strcpy(repo_handle.pds_name,repo_name);

	strcpy(repo_file,repo_name);
	strcat(repo_file,".dat");
	strcpy(ndx_file,repo_name);
	strcat(ndx_file,".ndx");

	repo_handle.pds_data_fp = (FILE *) fopen(repo_file, "wb");
	if( repo_handle.pds_data_fp == NULL ){
		perror(repo_file);
		return 1;
	}
	
	repo_handle.pds_ndx_fp = (FILE *) fopen(ndx_file, "wb");
	if( repo_handle.pds_ndx_fp == NULL )
	{
		perror(ndx_file);
		return 1;	
	}
	int i,j;

	for(i = 0;i<BLOCK_SIZE;i++)
	{
		repo_handle.ndx_block0.ndx[i].key = KEY_MIN + (i * KEY_BIN_SIZE);
		repo_handle.ndx_block0.ndx[i].offset = (i+1)*(sizeof(struct PDS_NdxBlock));
		repo_handle.ndx_block0.used_size+=1;
		
	}
	fwrite(&repo_handle.ndx_block0,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
	for(i=0;i<(MAX_BLOCKS-1);i++)
	{
		struct PDS_NdxBlock temp;
		temp.ndx_block_num = i+1;
		temp.used_size = 0;
		for(j=0;j<BLOCK_SIZE;j++)
		{
				temp.ndx[j].key = NULL_KEY;
				temp.ndx[j].offset = NULL_KEY;
		}
		fwrite(&temp,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
	}
	repo_handle.repo_create_status = PDS_REPO_CREATE;
	fclose(repo_handle.pds_data_fp);
	fclose(repo_handle.pds_ndx_fp);
	// reading the index file and build the binary tree
	return PDS_SUCCESS;
}


int pds_open( char *repo_name, int rec_size )
{
	char repo_file[30];
	char ndx_file[30];
	if( repo_handle.repo_status == PDS_REPO_OPEN || repo_handle.repo_create_status != PDS_REPO_CREATE)
		//return PDS_REPO_ALREADY_OPEN;
			return 1;
	strcpy(repo_handle.pds_name,repo_name);
	strcpy(repo_file,repo_name);	
	strcat(repo_file,".dat");
	strcpy(ndx_file,repo_name);
	strcat(ndx_file,".ndx");
	
	repo_handle.pds_data_fp = (FILE *) fopen(repo_file, "rb+");
	if( repo_handle.pds_data_fp == NULL ){
		perror(repo_file);
	}
	//repo_handle.ndx_blk.ndx_count = 0;
	repo_handle.pds_ndx_fp = (FILE *) fopen(ndx_file, "rb+");
	if( repo_handle.pds_data_fp == NULL ){
		perror(ndx_file);
	}
	//read the index file here and build your bst here!
	
	pds_load_primary_ndx();
	
	//print();
	
	repo_handle.repo_status = PDS_REPO_OPEN;
	repo_handle.rec_size = rec_size;
	
	return PDS_SUCCESS;
}
void print()
{
	int i =0;
	for(i =0;i<repo_handle.ndx_block0.used_size;i++)
	{
		printf("%d %d\n",repo_handle.ndx_block0.ndx[i].key, repo_handle.ndx_block0.ndx[i].offset);
	}
}
// Seek to the end of the data file
// Create an index entry with the current file location
// Add index entry to BST
// Write the record at the current file location
int compare (const void * a, const void * b)
{
	struct PDS_NdxInfo *c1 = (struct PDS_NdxInfo *)a;
	struct PDS_NdxInfo *c2 = (struct PDS_NdxInfo *)b;
  	if(c1->key > c2-> key)
  		return 1;
  	else if(c1->key > c2->key)
  		return -1;
  	else
  		return 0;
}

int put_rec_by_key( int key, void *rec )
{

	int offset, status;
	int dup;
	if( repo_handle.repo_status != PDS_REPO_OPEN || repo_handle.repo_create_status != PDS_REPO_CREATE || key < 1000)
		return 1;
	fseek(repo_handle.pds_data_fp,0,SEEK_END);

	offset = ftell(repo_handle.pds_data_fp);
	status = PDS_SUCCESS;
	// check for duplicates
	
	int block_num;
	struct PDS_NdxBlock ndx_blk;
	block_num = (key - KEY_MIN)/KEY_BIN_SIZE;

	if(key >= (repo_handle.ndx_block0.ndx[BLOCK_SIZE-1].key))
		block_num = BLOCK_SIZE - 1;

	fseek(repo_handle.pds_ndx_fp, repo_handle.ndx_block0.ndx[block_num].offset, SEEK_SET);
	fread(&ndx_blk,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
	
	if(ndx_blk.used_size >= BLOCK_SIZE )
	{
		//printf("NDX BLOCK FULL\n");		
		return 1;	
	}
	else
	{
		dup = search_ndx(key, &ndx_blk,0,ndx_blk.used_size-1);

		if(dup == -1)
		{
			ndx_blk.ndx[ndx_blk.used_size].key = key;
			ndx_blk.ndx[ndx_blk.used_size].offset = offset;
			ndx_blk.used_size += 1;

			qsort(ndx_blk.ndx,ndx_blk.used_size,sizeof(struct PDS_NdxInfo),compare);
			
			fseek(repo_handle.pds_ndx_fp, repo_handle.ndx_block0.ndx[block_num].offset, SEEK_SET);
			int j =0;
				
			fwrite(&ndx_blk,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
			fwrite(&key,sizeof(int),1,repo_handle.pds_data_fp);

			fwrite(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);

		}
		else
		{
			return 1;
		}
	}
	return PDS_SUCCESS;

}

int get_rec_by_key( int key, void *rec )
{
	struct PDS_NdxInfo *ndx_entry;
	
	//struct BST_Node *bst_node;
	int offset, status;
	if( repo_handle.repo_status != PDS_REPO_OPEN || repo_handle.repo_create_status != PDS_REPO_CREATE)
	{
		return PDS_REC_NOT_FOUND;
	}
	int block_num;
	struct PDS_NdxBlock ndx_blk;
	block_num = (key - KEY_MIN)/KEY_BIN_SIZE;

	if(key >= (repo_handle.ndx_block0.ndx[BLOCK_SIZE-1].key))
		block_num = BLOCK_SIZE - 1;

	fseek(repo_handle.pds_ndx_fp, repo_handle.ndx_block0.ndx[block_num].offset,SEEK_SET);
	fread(&ndx_blk,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
	int j =0;
	
	int chk_index;
	chk_index = search_ndx(key,&ndx_blk,0,ndx_blk.used_size-1);

	if(chk_index == -1)
	{
		status = PDS_REC_NOT_FOUND;
		return status;
	}
	else{
		
		offset = ndx_blk.ndx[chk_index].offset;
		fseek(repo_handle.pds_data_fp, offset, SEEK_SET);
		//read the key explicitly
		fread(&key, sizeof(int) , 1,repo_handle.pds_data_fp);
		fread(rec, repo_handle.rec_size, 1, repo_handle.pds_data_fp);
		status = PDS_SUCCESS;
	}
	return status;
}
int pds_load_primary_ndx()
{
	rewind(repo_handle.pds_ndx_fp);
	int index_read = fread(&repo_handle.ndx_block0,sizeof(struct PDS_NdxBlock),1,repo_handle.pds_ndx_fp);
	//repo_handle.ndx_blk.ndx_count = index_read;
}
int pds_close()
{
	if( repo_handle.repo_status != PDS_REPO_OPEN || repo_handle.repo_create_status != PDS_REPO_CREATE)
		return 1;
	//printf("came here\n");
	strcpy(repo_handle.pds_name, "");
	fclose(repo_handle.pds_data_fp);
	
	fseek(repo_handle.pds_ndx_fp, 0, SEEK_SET);	
	fwrite(&repo_handle.ndx_block0, sizeof(struct PDS_NdxBlock), 1, repo_handle.pds_ndx_fp);
	
	fclose(repo_handle.pds_ndx_fp);
	//here save your bst into a file!
	repo_handle.repo_status = PDS_REPO_CLOSED;
	return PDS_SUCCESS;
}
