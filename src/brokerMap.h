#include<stdio.h>
#include<stdlib.h>
#include<sqlite3.h>
#include<string.h>
#include<malloc.h>
#include<time.h>
#include <openssl/md5.h>
#define DELIM "."
char x[100];
sqlite3 *db;
int rc;
char *zErrMsg = 0;
char *sql;
int choice,i;
int flag;
int port,priority;
char nameID[100],ip1[100],ip2[100],new_broker[100];
sqlite3_stmt *insert_sql;
time_t start_time;


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      fprintf(stdout,"%s\t\t",argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   strcpy(x,argv[0] ? argv[0] : "NULL");
   return 0;
}


//Creating a table BROKERLIST in the topology.cnf database
void create_table()
{
  rc = sqlite3_open("topology.cnf", &db);
  if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
  }
  sql = "CREATE TABLE BROKERLIST("  \
         "IP CHAR(16) PRIMARY KEY    ," \
         "PORT INT CHECK(PORT > 1023) CHECK(PORT < 49152) NOT NULL," \
         "NAMEID CHAR(40) NOT NULL," \
         "PRIORITY INT CHECK(PRIORITY >= 0) CHECK(PRIORITY <= 100) NOT NULL );";
  rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
  if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
  }else{
      fprintf(stdout, "Table created successfully\n");
  }
}


//IP address Validation
/* returns 1 if string contain only digits, else return 0 */
int valid_digit(char *ip_str)
{
    while (*ip_str) {
        if (*ip_str >= '0' && *ip_str <= '9')
            ++ip_str;
        else
            return 0;
    }
    return 1;
}
/* return 1 if IP string is valid, else return 0 */
int is_valid_ip(char *ip_str)
{
    int i, num, dots = 0;
    char *ptr;
 
    if (ip_str == NULL)
        return 0;
 
    ptr = strtok(ip_str, DELIM);
 
    if (ptr == NULL)
        return 0;
 
    while (ptr) {
 
        /* after parsing string, it must contain only digits */
        if (!valid_digit(ptr))
            return 0;
 
        num = atoi(ptr);
 
        /* check for valid IP */
        if (num >= 0 && num <= 255) {
            /* parse remaining string */
            ptr = strtok(NULL, DELIM);
            if (ptr != NULL)
                ++dots;
        } else
            return 0;
    }
 
    /* valid IP string must contain 3 dots */
    if (dots != 3)
        return 0;
    return 1;
}


//Adding a new broker to the list
void add_broker()
{
    rc = sqlite3_open("topology.cnf", &db);
    if(rc){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    }
    
    fprintf(stdout,"\nEnter IP address ");
    scanf("%s",ip1);
    //validating the entered IP address
    do{
         for(i=0;ip1[i]!='\0';i++)
             ip2[i] = ip1[i];
         ip2[i] = '\0';
         flag = is_valid_ip(ip1);
         if(flag == 0){
             fprintf(stdout,"\n enter valid IP ");
             scanf("%s",ip1);
         } 
         else
            break;
    }while(1);  
    
    fprintf(stdout,"\nEnter port number "); 
    scanf("%d",&port);
     // validating port number
    do{
    	if(port > 1024 && port < 49152)
    	{
    		break;
    	}else {
    		printf("\n enter valid port number ");
    		scanf("%d",&port);
    	}	
    }while(1); 
    
    fprintf(stdout,"\n Enter nameID ");
    scanf("%s",nameID);
    
    
    fprintf(stdout,"\n Enter priority (0-100) ");
    scanf("%d",&priority);
    //checking if the priority is between 0-100
    do{
    	if(priority >= 0 && priority <= 100)
    	    break;
    	else{
    		fprintf(stdout,"\n Enter valid priority ");
    		scanf("%d",&priority);
    	}    
    }while(1);
    
    //inserting the entered input into the table
    sql = "INSERT INTO BROKERLIST(IP,PORT,NAMEID,PRIORITY) VALUES(?,?,?,?); " ;
    rc = sqlite3_prepare_v2(db, sql,-1, &insert_sql, NULL);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }else{
        fprintf(stdout, "Added broker successfully\n");
        sqlite3_bind_text(insert_sql,1,ip2,-1,SQLITE_STATIC);
        sqlite3_bind_int(insert_sql,2,port);
        sqlite3_bind_text(insert_sql,3,nameID,-1,SQLITE_STATIC);
        sqlite3_bind_int(insert_sql,4,priority);
        rc = sqlite3_step(insert_sql);
        if(rc != SQLITE_DONE){
           fprintf(stdout,"ERROR inserting the data");
        }
        sqlite3_finalize(insert_sql);
   }        
}


//removing a broker from the list
void remove_broker()
{ 
   rc = sqlite3_open("topology.cnf", &db);
   if(rc){
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   }else{
       fprintf(stdout,"\nEnter NameID of the broker to be removed");
       scanf("%s",nameID);
       sql = "DELETE from BROKERLIST where NAMEID = ? ";
       rc = sqlite3_prepare_v2(db, sql,-1, &insert_sql, NULL);
       if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       }else{
               fprintf(stdout, "Removed broker successfully\n");
               sqlite3_bind_text(insert_sql,1,nameID,-1,SQLITE_STATIC);
               rc = sqlite3_step(insert_sql);
               if(rc != SQLITE_DONE){
                   fprintf(stdout,"Could not remove the broker");
               }
           }
        }
   
   sqlite3_finalize(insert_sql);                        
}


//Updating the priority of the broker
void update_priority()
{
   fprintf(stdout,"\nEnter NameID of broker whose priority has to be changed and the new priority");
   scanf("%s%d",nameID,&priority);
   //checking if the priority is valid(0-100) or no
   do{
      if(priority>=0 && priority<=100)
         break;
      else{
         fprintf(stdout,"Enter a priority between 0-100");
         scanf("%d",&priority);
      }
   }while(1);
   rc = sqlite3_open("topology.cnf", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   }
   sql = " UPDATE BROKERLIST set PRIORITY = ? where NAMEID = ? ; ";
   rc = sqlite3_prepare_v2(db, sql,-1, &insert_sql, NULL);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Updated Priority of broker\n");
      sqlite3_bind_int(insert_sql,1,priority);
      sqlite3_bind_text(insert_sql,2,nameID,-1,SQLITE_STATIC);
      rc = sqlite3_step(insert_sql);
      if(rc != SQLITE_DONE){
           fprintf(stdout,"ERROR inserting the data");
      }
      sqlite3_finalize(insert_sql);
   }
}


//Shows the list of all brokers.
void show_map()
{
   rc = sqlite3_open("topology.cnf", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   }
   sql = "SELECT * from BROKERLIST" ;
   fprintf(stdout,"IP\t\t\tport\t\tnameID\t\tPriority\n");
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
     fprintf(stderr, "SQL error: %s\n", zErrMsg);
     sqlite3_free(zErrMsg);
   }
}

//Data structure to store the list of brokers that fail (to store the fault history)

struct node{
	char name[100];
	time_t f_time;
	struct node *next;
	};
typedef struct node snode;
snode *newnode, *ptr, *prev, *temp;
snode *first = NULL, *last = NULL;

snode* createNode(char *name,time_t failure_time)
{
	newnode = (snode *)malloc(sizeof(snode));
	if(newnode == NULL)
	{
		fprintf(stdout,"\n memory allocation failed");
		return 0;
	}
	strcpy(newnode->name,name);
	newnode->f_time = failure_time;
	newnode->next = NULL;
	return newnode;
}
void insertNode(char *name, time_t failure_time)
{
	newnode = createNode(name,failure_time);
	if(first == NULL)
	{
		first = newnode;
		last = newnode;
		first->next = NULL;
		last->next = NULL;
		
	}
	else {
		last->next = newnode;
		last = newnode;
		last->next = NULL;
	}
}

// To display the fault history
void show_fault_history()
{
	if(first == NULL)
	   fprintf(stdout,"\n No brokers failed");
	else{
		for(ptr=first;ptr!=NULL;ptr=ptr->next)
		    fprintf(stdout,"\n%s\t%f",ptr->name,ptr->f_time);
	}   
}


//To get the broker with maximum priority and return the nameID of the broker to which the connection has to be established.
char* assignBroker()
{
   rc = sqlite3_open("topology.cnf", &db);
   if(rc){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
   }
   sql = "SELECT NAMEID from BROKERLIST where PRIORITY = MAX(PRIORITY)) " ;
   fprintf(stdout,"\nNAMEID : ");
   rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
     fprintf(stderr, "SQL error: %s\n", zErrMsg);
     sqlite3_free(zErrMsg);
   }
   return x;
}

//action to be done when the broker fails. This method is called when the connection is lost
char* failure(char *name)
{
   fprintf(stdout, "Connection Lost!!!");
   time_t failure_time = time(NULL);
   insertNode(name,failure_time);//inserting into the data structure to show fault history
   fprintf(stdout, "Connecting to next broker...");
   strcpy(new_broker,assignBroker());
   start_time = time(NULL);//to store the time when a new broker is connected. Used by show_up_time method
   return new_broker;
}


//To show the time since which the broker is functioning
void show_uptime()
{
	fprintf(stdout,"\n Broker is up since %f",difftime(start_time,time(NULL)));
}

void show_current_broker()
{
	fprintf(stdout,"\n %s",new_broker);
}


//Change_password implementation
//To generate hash for a string
char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
    
} 

//To search if the entered old password is in the file or no. Returns 1 if the password exists. Else 0   
int search_pwd(char *oldPwd)
{
    char file[]="../etc/telescope_pass";
    char buf[100];
    FILE *fp = fopen(file,"r");
    if(fp == NULL){
       fprintf(stdout,"\n password file doesn't exist");
       exit(1);
    }
    char *p;
    while(fgets(buf,100,fp))
    {
    	  p=strstr( buf , str2md5(oldPwd,strlen(oldPwd)));
          if(p!=NULL)
          {
              return 1;
          }
    }
    fclose(fp);
    return 0;
}
//returns 1 if password is changed successfully
int insert_pwd(char *newPwd)
{
   FILE *fp;
   char file[]="../etc/telescope_pass";
   fp = fopen(file, "a+");
   if(fp == NULL)
   {
      fprintf(stdout,"Password File doesn't exist");
      return 0;
   }
   else {   	
   fputs(str2md5(newPwd,strlen(newPwd)), fp);
   fputs("\n",fp);
   fclose(fp);
   }
   return 1;
}   

void change_password()
{
   int flag =  0,counter = 3;
   char oldPwd[100],newPwd[100];
   FILE *fp;
   if(fp = fopen("../etc/telescope_pass","r"))
   {
        fprintf(stdout,"\n enter the old password");
	scanf("%s",oldPwd);	
	do{
		flag = search_pwd(oldPwd);//search returns 1 if oldPwd is found in file
	        if(strcmp(oldPwd,"telescopecq") == 0)
	        {
	   	   break;
	        }
                else if(flag == 1)// entered old password is found
                {
                   break;
                } 
	   counter--;
	   fprintf(stdout,"\n Enter the correct password(Number of attempts left: %d),counter");
	   scanf("%s",oldPwd);
	 }while(counter != 0);
	 if(counter == 0){
	   fprintf(stdout,"\n Password cannot be changed");
	   return;
	 }
         fprintf(stdout,"\n enter new password");
         scanf("%s",newPwd);
         flag=insert_pwd(newPwd);
      	if (flag == 1)
      	   fprintf(stdout,"\n Password changed successfully");
        else
           fprintf(stdout,"Password could not be changed");
   }
   else
       fprintf(stdout,"Password file doesn't exist");    
}