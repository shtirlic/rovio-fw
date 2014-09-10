#include "wb_syslib_addon.h"
#include "hic_host.h"
#include "FTH_Int.h"
#include "RemoteFunc.h"
#include "RemoteNet.h"
#include "clk.h"
#include "pll.h"
#include "tt_thread.h"

#include "RemoteFuncTest.h"
#include "test_recvmsg.h"

void test_recvmsg(void)
{
	test_printf_begin("test_recvmsg");
	test_recvmsg_entry();
	test_printf_end("test_recvmsg");
}

static void test_tcp_socket_thread_join(int ithread_handle)
{
	cyg_thread_info	info;
	cyg_uint16		id = 0;
	cyg_bool		rc;
	
	cyg_handle_t	next_thread = 0;
	cyg_uint16		next_id = 0;
	
	while(cyg_thread_get_next(&next_thread, &next_id) != false)
	{
		if(ithread_handle == next_thread)
		{
			id = next_id;
			break;
		}
	}
	
	while(1)
	{
		rc = cyg_thread_get_info(ithread_handle, id, &info);
		if(rc == false)
		{
			printf("Error get thread %d info\n", ithread_handle);
			break;
		}
		else
		{
			if((info.state & EXITED) != 0)
			{
				break;
			}
		}
		cyg_thread_yield();
	}
}

static char msg[TEST_RECVMSG_SERVER_NUM][TEST_RECVMSG_MSG_LEN];
static TEST_RECVMSG_DATA_T netdata[TEST_NETREAD_SERVER_NUM];

void test_recvmsg_entry(void)
{
	int iPort = TEST_RECVMSG_SERVER_BEGIN_PORT;
	int i;
	
	for(i = 0; i < TEST_RECVMSG_SERVER_NUM; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].precvmsgbuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_recvmsg_server, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	tt_msleep(1000);
	
	for(; i < TEST_RECVMSG_SERVER_NUM * 2; i++)
	{
		netdata[i].iport = iPort+i;
		netdata[i].pbuf = (char*)NON_CACHE(g_RemoteNet_Buf1[i]);
		netdata[i].precvmsgbuf = msg[i];
		cyg_thread_create(THREAD_PRIORITY, &test_recvmsg_client, (cyg_addrword_t)&netdata[i], 
						NULL, thread_stack[i], STACK_SIZE, &thread_handle[i], &thread[i]);
		cyg_thread_resume(thread_handle[i]);
	}
	
	for(i = 0; i < TEST_RECVMSG_SERVER_NUM * 2; i++)
	{
		test_tcp_socket_thread_join(thread_handle[i]);
		cyg_thread_delete(thread_handle[i]);
	}
}

void test_recvmsg_server(cyg_addrword_t pnetdata)
{
	int s, new_s, i, recvmsglen;
	struct sockaddr_in sa, r_sa;
	int r_sa_l = sizeof(r_sa);
	struct hostent *hp;
	int ierr = 0;
	struct msghdr msghdr_msg;
	struct iovec *piov;
	int j;
	int perIov_len;
	
	int threadid;
		
	int port = ((TEST_RECVMSG_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_RECVMSG_DATA_T*)pnetdata)->pbuf;
	//char *precvmsgbuf = ((TEST_RECVMSG_DATA_T*)pnetdata)->precvmsgbuf;
	
	//threadid = cyg_thread_self();
	threadid = port;
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_recvmsg_server");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_recvmsg_server");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_recvmsg_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_recvmsg_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}

	if(listen(s, 10, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_recvmsg_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if((new_s = accept(s, (struct sockaddr*)&sa, (size_t*)&r_sa_l, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_recvmsg_server");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	perIov_len = TEST_RECVMSG_MSG_PART_LEN;
	piov = malloc(TEST_RECVMSG_MSG_PARTS*sizeof(struct iovec));
	if(piov == NULL)
	{
		ierr = 1;
		goto fail;
	}
	memset(piov, 0, TEST_RECVMSG_MSG_PARTS*sizeof(struct iovec));
		
	for(j = 0; j < TEST_RECVMSG_MSG_PARTS; j++)
	{
		piov[j].iov_base = malloc(perIov_len);
		if(piov[j].iov_base == NULL)
		{
			for(i = 0; i < j; i++) free(piov[j].iov_base);
			ierr = 1;
			goto fail;
		}
		piov[j].iov_len = perIov_len;
	}
		
	msghdr_msg.msg_name = NULL;
	msghdr_msg.msg_namelen = 0;
	msghdr_msg.msg_iov = piov;
	msghdr_msg.msg_iovlen = TEST_RECVMSG_MSG_PARTS;
	msghdr_msg.msg_control = NULL;
	msghdr_msg.msg_controllen = 0;
	msghdr_msg.msg_flags = 0;
	
	recvmsglen = recvmsg(-1, &msghdr_msg, 0, pbuf, RNT_BUFFER_LEN);
	if(recvmsglen >= 0)
	{
		ierr = 1;
		goto fail;
	}
	
	recvmsglen = recvmsg(new_s, NULL, 0, pbuf, RNT_BUFFER_LEN);
	if(recvmsglen >= 0)
	{
		ierr = 1;
		goto fail;
	}
	
	recvmsglen = recvmsg(new_s, &msghdr_msg, -1, pbuf, RNT_BUFFER_LEN);
	if(recvmsglen >= 0)
	{
		ierr = 1;
		goto fail;
	}
	
	recvmsglen = recvmsg(new_s, &msghdr_msg, 0, NULL, RNT_BUFFER_LEN);
	if(recvmsglen >= 0)
	{
		ierr = 1;
		goto fail;
	}
	
	recvmsglen = recvmsg(new_s, &msghdr_msg, 0, pbuf, 0);
	if(recvmsglen >= 0)
	{
		ierr = 1;
		goto fail;
	}
	
	{
		struct msghdr msghdr_msg_test;
		struct iovec *piov_test;
		
		memcpy(&msghdr_msg_test, &msghdr_msg, sizeof(msghdr_msg_test));
		msghdr_msg_test.msg_iov = NULL;
		recvmsglen = recvmsg(new_s, &msghdr_msg_test, 0, pbuf, RNT_BUFFER_LEN);
		if(recvmsglen >= 0)
		{
			ierr = 1;
			goto fail;
		}
		
		// this is like recv 0, also would block
		/*
		memcpy(&msghdr_msg_test, &msghdr_msg, sizeof(msghdr_msg_test));
		msghdr_msg_test.msg_iovlen = 0;
		recvmsglen = recvmsg(new_s, &msghdr_msg_test, 0, pbuf, RNT_BUFFER_LEN);
		if(recvmsglen >= 0)
		{
			test_printf_error("test_recvmsg_server");
			ierr = 1;
			goto fail;
		}
		*/
		
		memcpy(&msghdr_msg_test, &msghdr_msg, sizeof(msghdr_msg_test));
		piov_test = malloc(TEST_RECVMSG_MSG_PARTS*sizeof(struct iovec));
		memcpy(piov_test, piov, TEST_RECVMSG_MSG_PARTS*sizeof(struct iovec));
		piov_test[0].iov_base = NULL;
		msghdr_msg_test.msg_iov = piov_test;
		recvmsglen = recvmsg(new_s, &msghdr_msg_test, 0, pbuf, RNT_BUFFER_LEN);
		if(recvmsglen >= 0)
		{
			ierr = 1;
			goto fail;
		}
		if(piov_test != NULL) free(piov_test);
	}
	
fail:
	for(j = 0; j < TEST_RECVMSG_MSG_PARTS; j++)
	{
		if(piov[j].iov_base != NULL) free(piov[j].iov_base);
	}
	if(piov != NULL) free(piov);
	
	if(ierr == 0)
		test_printf_success("test_recvmsg_server");
	else
		test_printf_error("test_recvmsg_server");
	
	netclose(new_s, pbuf, RNT_BUFFER_LEN);
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}

void test_recvmsg_client(cyg_addrword_t pnetdata)
{
	int s, readlen;
	struct sockaddr_in sa, r_sa;
	struct hostent *hp;
	
	int threadid;
		
	int port = ((TEST_RECVMSG_DATA_T*)pnetdata)->iport;
	char *pbuf = ((TEST_RECVMSG_DATA_T*)pnetdata)->pbuf;
	char *precvmsgbuf = ((TEST_RECVMSG_DATA_T*)pnetdata)->precvmsgbuf;
	
	threadid = cyg_thread_self();
	
	if((hp = gethostbyname(TEST_REMOTEFUNC_HOSTNAME, pbuf, RNT_BUFFER_LEN)) == NULL)
	{
		test_printf_error("test_recvmsg_client");
		cyg_thread_exit();
	}
	
	memcpy(&(r_sa.sin_addr), hp->h_addr_list0, hp->h_length);
	r_sa.sin_family = AF_INET;
	r_sa.sin_port = htons(IPPORT_USERRESERVED + port);
	
	memcpy(&(sa.sin_addr), hp->h_addr_list0, hp->h_length);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(IPPORT_USERRESERVED + port - TEST_RECVMSG_SERVER_NUM);
	
	if((s = socket(AF_INET, SOCK_STREAM, PF_UNSPEC, pbuf, RNT_BUFFER_LEN)) == -1)
	{
		test_printf_error("test_recvmsg_client");
		cyg_thread_exit();
	}
	
	if(set_reuseaddr(s, pbuf, RNT_BUFFER_LEN) == -1)
	{
		test_printf_error("test_recvmsg_client");
		netclose(s, pbuf, RNT_BUFFER_LEN);
		cyg_thread_exit();
	}
	
	if(bind(s, (struct sockaddr*)&r_sa, sizeof(r_sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_recvmsg_client");
		cyg_thread_exit();
	}

	if(connect(s, (struct sockaddr*)&sa, sizeof(sa), pbuf, RNT_BUFFER_LEN) == -1)
	{
		netclose(s, pbuf, RNT_BUFFER_LEN);
		test_printf_error("test_recvmsg_client");
		cyg_thread_exit();
	}
	
	readlen = recv(s, precvmsgbuf, TEST_RECVMSG_MSG_LEN, 0, pbuf, RNT_BUFFER_LEN);
	netclose(s, pbuf, RNT_BUFFER_LEN);
	cyg_thread_exit();
}
