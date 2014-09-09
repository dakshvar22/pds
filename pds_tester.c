#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "pds.h"
#include "contact.h"

#define TREPORT(a1,a2,a3,a4,a5) printf("%s:%s %s %s %s\n",a1,a2,a3,a4,a5); fflush(stdout);

void process_line( char *test_case );

main(int argc, char *argv[])
{
	FILE *cfptr;
	char test_case[50];

	if( argc != 2 ){
		fprintf(stderr, "Usage: %s testcasefile\n", argv[0]);
		exit(1);
	}

	cfptr = (FILE *) fopen(argv[1], "r");
	while(fgets(test_case, sizeof(test_case)-1, cfptr)){
		//printf("line:%s",test_case);
		if( !strcmp(test_case,"\n") || !strcmp(test_case,"") )
			continue;
		process_line( test_case );
	}
}

void process_line( char *test_case )
{
	char repo_name[30];
	char command[10], param1[10], param2[10], info[100];
	int contact_id, status, rec_size, expected_status;
	struct Contact testContact;

	strcpy(testContact.contact_name, "dummy name");
	strcpy(testContact.phone, "dummy number");

	rec_size = sizeof(struct Contact);

	sscanf(test_case, "%s%s%s", command, param1, param2);
	//printf("Test case: %s-%s-%s\n", command, param1, param2);
	if( !strcmp(command,"OPEN") ){
		strcpy(repo_name, param1);
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		status = pds_open( repo_name, rec_size );
		if( status == expected_status ){
			TREPORT("PASS", command, param1, param2, "");
		}
		else{
			TREPORT("FAIL", command, param1, param2, "");
		}
	}
	else if( !strcmp(command,"STORE") ){
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%d", &contact_id);
		testContact.contact_id = contact_id;
		status = add_contact( &testContact );
		if( status == expected_status ){
			TREPORT("PASS", command, param1, param2, "");
		}
		else{
			TREPORT("FAIL", command, param1, param2, "");
		}
	}
	else if( !strcmp(command,"SEARCH") ){
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		sscanf(param1, "%d", &contact_id);
		testContact.contact_id = -1;
		status = search_contact( contact_id, &testContact );
		if( status != expected_status ){
			sprintf(info,"search key: %d; Got status %d",contact_id, status);
			TREPORT("FAIL", command, param1, param2, info);
		}
		else{
			// Check if the retrieved values match
			if( expected_status == 1 || (testContact.contact_id == contact_id && 
					strcmp(testContact.contact_name,"dummy name") == 0 &&
					strcmp(testContact.phone,"dummy number") == 0)){
				TREPORT("PASS", command, param1, param2, "");
			}
			else{
				sprintf(info,"Contact data not matching... Expected:{%d,%s,%s} Got:{%d,%s,%s}\n",
					contact_id, "dummy_name", "dummy number", 
					testContact.contact_id, testContact.contact_name, testContact.phone
				);
				TREPORT("FAIL", command, param1, param2, info);
			}
		}
	}
	else if( !strcmp(command,"CLOSE") ){
		if( !strcmp(param1,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		status = pds_close();
		if( status == expected_status ){
			TREPORT("PASS", command, param1, "", "");
		}
		else{
			TREPORT("FAIL", command, param1, "", "");
		}
	}
	else if( !strcmp(command,"CREATE") )
	{
		strcpy(repo_name, param1);
		if( !strcmp(param2,"0") )
			expected_status = CONTACT_SUCCESS;
		else
			expected_status = CONTACT_FAILURE;

		status = pds_create( repo_name);
		if( status == expected_status ){
			TREPORT("PASS", command, param1, param2, "");
		}
		else{
			TREPORT("FAIL", command, param1, param2, "");
		}
	}
}

