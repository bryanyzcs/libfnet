#include    <unistd.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/param.h>
#include    <stdlib.h>
#include    <string.h>
#include    <errno.h>
#include    <stdio.h>
#include    <stdlib.h>

#include    "include/dispatch.h"
#include    "include/proto.h"
#include    "include/error.h"

extern int cmsockfd;
int flag_init_rfs = 0;
extern int flag_init_cm;
int rfsockfd;
struct sockaddr server_addr_d;



int 
init_receive_feature_service(){
    //struct sockaddr_in *sd_addr = (struct sockaddr_in*)&server_addr_d;
    if(flag_init_cm){
        rfsockfd = cmsockfd;
    }
    else{
        return -1;
    }
    

    /*memset(sd_addr, 0, sizeof (*sd_addr));
    sd_addr->sin_family = AF_INET;
    sd_addr->sin_port = htons(SERVER_FEATURE_DISTRIBUTE_PORT);
    if(inet_pton(AF_INET, SERVER_FEATURE_DISTRIBUTE_IPV4, &(sd_addr->sin_addr.s_addr)) < 0){
        err_sys("inet_pton error");
        return -1;
    }*/
    flag_init_rfs = 1;
    return 0;

}

int 
dispatch(feature_handler fhandler, unsigned char * fhdl_args){
    if(flag_init_rfs == 0){
        init_receive_feature_service();
    }

    char ft_msg[MAX_UDP_MSG];
    int n;
    socklen_t addr_len;
    struct feature_set *fts;
    
    for( ; ; ){
        printf("%d", rfsockfd);
        n = recvfrom(rfsockfd, ft_msg, MAX_UDP_MSG, 0, &server_addr_d, &addr_len);

        if(n < 0){
            err_sys("recvfrom error");
            continue;
        }
        printf("Receive a packet from %s:%d\n", inet_ntoa(((struct sockaddr_in*)&server_addr_d)->sin_addr), ntohs(((struct sockaddr_in*)&server_addr_d)->sin_port));

        // parse the packet as a feature packet
        short proto_len;
        int msg_offset;

        if(ft_msg[0] != FEATURE || ft_msg[1] != GENERAL_CODE){
            err_msg("feature packet error");
            continue;
        }

        proto_len = *(short *)(ft_msg + 2);
        if(proto_len <= 4){
            err_msg("feature packet length error");
            continue;
        }
        msg_offset = 4;

        fts = (struct feature_set*)malloc(sizeof (struct feature_set));
        fts->no_ft = 0;

        empty_feature_set(fts);
        

        do{
            struct feature *ft = (struct feature*)malloc(sizeof (struct feature)); 
            if(ft == NULL){
                err_msg("malloc error");
                return -1;
            }
            ft->ft_code= *(ft_msg + msg_offset);
            ft->ft_len = *(short *)(ft_msg+msg_offset + 1);
            char * val = (char *)malloc(ft->ft_len + 1);
            memcpy(val, ft_msg + msg_offset + 3, ft->ft_len);
            val[ft->ft_len] = '\0';
            // printf("%d:%s\n", ft->ft_code, val);
            ft->ft_val = val;
            msg_offset += 3 + ft->ft_len;
            fts->f_feature[ft->ft_code] = 1;
            fts->features[ft->ft_code] = ft;
            fts->no_ft++;
            
        }while(msg_offset < proto_len);
        if(fhandler != NULL){
            fhandler(fhdl_args, fts);
        }
    }
}
