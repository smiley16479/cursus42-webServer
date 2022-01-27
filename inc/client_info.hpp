#ifndef CLIENT_INFO_HPP
# define CLIENT_INFO_HPP

#include <sys/socket.h>	// pour socket()
#include <arpa/inet.h>
#include <cstring>
#include "request_handler.hpp"
#include "struct_webserv.hpp"

class client_info {
	public:
		int			com_socket;
		int			loc_fd[2];
		pid_t		cgi_pid;
		std::string	loc_path;
		size_t		mode;
		int			flag;
		std::string	resp;
		std::string rqst;
		std::string	chunk_buffer;
		size_t		chunk_mode;
		size_t		rq_mode;
		std::string post_boundary;
		time_t		rqst_time_start;
		size_t		_cLen;
		int			time_out;
		std::string	addr;
		struct struct_epoll* _epoll;

		void	fd_in(request_handler& header);
		void	fd_out(request_handler& header);

		void	solve_request(request_handler& header);

		void	recv_handler(request_handler& header);
		void	read_handler(request_handler& header);

		void	write_handler(request_handler& header);
		void	send_handler(request_handler& header);
		void	cgi_write_handler(request_handler& header);
		void	cgi_resp_handler(request_handler& header);

		void	chunked_handler(request_handler& header);

		void	compute(request_handler& header);

		bool	get_rq_type();

		bool	is_request_fulfilled();
		bool	is_multipart_rqst_fulfilled();
		bool	is_clen_rqst_fulfilled();
		bool	is_chunked_rqst_fulfilled();

		void	set_recv_mode();
		void	set_send_mode();

		void 	time_reset();
		void 	remove();
};

typedef void	(client_info::*t_func)(request_handler&);

#endif
