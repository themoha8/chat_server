#ifndef ERRNO_H_SENTRY
#define ERRNO_H_SENTRY

typedef struct error_t {
	int code;
	const char *msg;
} error;

enum { max_err_idx = 133 };

#define SUCCESS		 0
#define EPERM        1			/* Operation not permitted */
#define ENOENT       2			/* No such file or directory */
#define ESRCH        3			/* No such process */
#define EINTR        4			/* Interrupted system call */
#define EIO      	 5			/* I/O error */
#define ENXIO        6			/* No such device or address */
#define E2BIG        7			/* Argument list too long */
#define ENOEXEC      8			/* Exec format error */
#define EBADF        9			/* Bad file number */
#define ECHILD      10			/* No child processes */
#define EAGAIN      11			/* Try again */
#define ENOMEM      12			/* Out of memory */
#define EACCES      13			/* Permission denied */
#define EFAULT      14			/* Bad address */
#define ENOTBLK     15			/* Block device required */
#define EBUSY       16			/* Device or resource busy */
#define EEXIST      17			/* File exists */
#define EXDEV       18			/* Cross-device link */
#define ENODEV      19			/* No such device */
#define ENOTDIR     20			/* Not a directory */
#define EISDIR      21			/* Is a directory */
#define EINVAL      22			/* Invalid argument */
#define ENFILE      23			/* File table overflow */
#define EMFILE      24			/* Too many open files */
#define ENOTTY      25			/* Not a typewriter */
#define ETXTBSY     26			/* Text file busy */
#define EFBIG       27			/* File too large */
#define ENOSPC      28			/* No space left on device */
#define ESPIPE      29			/* Illegal seek */
#define EROFS       30			/* Read-only file system */
#define EMLINK      31			/* Too many links */
#define EPIPE       32			/* Broken pipe */
#define EDOM        33			/* Math argument out of domain of func */
#define ERANGE      34			/* Math result not representable */

#define EDEADLK     35			/* Resource deadlock would occur */
#define ENAMETOOLONG 36			/* File name too long */
#define ENOLCK      37			/* No record locks available */
#define ENOSYS      38			/* Invalid system call number */
#define ENOTEMPTY   39			/* Directory not empty */
#define ELOOP       40			/* Too many symbolic links encountered */
#define ENOMSG      42			/* No message of desired type */
#define EIDRM       43			/* Identifier removed */
#define ECHRNG      44			/* Channel number out of range */
#define EL2NSYNC    45			/* Level 2 not synchronized */
#define EL3HLT      46			/* Level 3 halted */
#define EL3RST      47			/* Level 3 reset */
#define ELNRNG      48			/* Link number out of range */
#define EUNATCH     49			/* Protocol driver not attached */
#define ENOCSI      50			/* No CSI structure available */
#define EL2HLT      51			/* Level 2 halted */
#define EBADE       52			/* Invalid exchange */
#define EBADR       53			/* Invalid request descriptor */
#define EXFULL      54			/* Exchange full */
#define ENOANO      55			/* No anode */
#define EBADRQC     56			/* Invalid request code */
#define EBADSLT     57			/* Invalid slot */

#define EDEADLOCK   EDEADLK		/* Alias for EDEADLK */

#define EBFONT      59			/* Bad font file format */
#define ENOSTR      60			/* Device not a stream */
#define ENODATA     61			/* No data available */
#define ETIME       62			/* Timer expired */
#define ENOSR       63			/* Out of streams resources */
#define ENONET      64			/* Machine is not on the network */
#define ENOPKG      65			/* Package not installed */
#define EREMOTE     66			/* Object is remote */
#define ENOLINK     67			/* Link has been severed */
#define EADV        68			/* Advertise error */
#define ESRMNT      69			/* Srmount error */
#define ECOMM       70			/* Communication error on send */
#define EPROTO      71			/* Protocol error */
#define EMULTIHOP   72			/* Multihop attempted */
#define EDOTDOT     73			/* RFS specific error */
#define EBADMSG     74			/* Not a data message */
#define EOVERFLOW   75			/* Value too large for defined data type */
#define ENOTUNIQ    76			/* Name not unique on network */
#define EBADFD      77			/* File descriptor in bad state */
#define EREMCHG     78			/* Remote address changed */
#define ELIBACC     79			/* Can not access a needed shared library */
#define ELIBBAD     80			/* Accessing a corrupted shared library */
#define ELIBSCN     81			/* .lib section in a.out corrupted */
#define ELIBMAX     82			/* Attempting to link in too many shared libraries */
#define ELIBEXEC    83			/* Cannot exec a shared library directly */
#define EILSEQ      84			/* Illegal byte sequence */
#define ERESTART    85			/* Interrupted system call should be restarted */
#define ESTRPIPE    86			/* Streams pipe error */
#define EUSERS      87			/* Too many users */
#define ENOTSOCK    88			/* Socket operation on non-socket */
#define EDESTADDRREQ 89			/* Destination address required */
#define EMSGSIZE    90			/* Message too long */
#define EPROTOTYPE  91			/* Protocol wrong type for socket */
#define ENOPROTOOPT 92			/* Protocol not available */
#define EPROTONOSUPPORT 93		/* Protocol not supported */
#define ESOCKTNOSUPPORT 94		/* Socket type not supported */
#define EOPNOTSUPP  95			/* Operation not supported on transport endpoint */
#define EPFNOSUPPORT 96			/* Protocol family not supported */
#define EAFNOSUPPORT 97			/* Address family not supported by protocol */
#define EADDRINUSE  98			/* Address already in use */
#define EADDRNOTAVAIL 99		/* Cannot assign requested address */
#define ENETDOWN    100			/* Network is down */
#define ENETUNREACH 101			/* Network is unreachable */
#define ENETRESET   102			/* Network dropped connection because of reset */
#define ECONNABORTED 103		/* Software caused connection abort */
#define ECONNRESET  104			/* Connection reset by peer */
#define ENOBUFS     105			/* No buffer space available */
#define EISCONN     106			/* Transport endpoint is already connected */
#define ENOTCONN    107			/* Transport endpoint is not connected */
#define ESHUTDOWN   108			/* Cannot send after transport endpoint shutdown */
#define ETOOMANYREFS 109		/* Too many references: cannot splice */
#define ETIMEDOUT   110			/* Connection timed out */
#define ECONNREFUSED 111		/* Connection refused */
#define EHOSTDOWN   112			/* Host is down */
#define EHOSTUNREACH 113		/* No route to host */
#define EALREADY    114			/* Operation already in progress */
#define EINPROGRESS 115			/* Operation now in progress */
#define ESTALE      116			/* Stale file handle */
#define EUCLEAN     117			/* Structure needs cleaning */
#define ENOTNAM     118			/* Not a XENIX named type file */
#define ENAVAIL     119			/* No XENIX semaphores available */
#define EISNAM      120			/* Is a named type file */
#define EREMOTEIO   121			/* Remote I/O error */
#define EDQUOT      122			/* Quota exceeded */

#define ENOMEDIUM   123			/* No medium found */
#define EMEDIUMTYPE 124			/* Wrong medium type */
#define ECANCELED   125			/* Operation canceled */
#define ENOKEY      126			/* Required key not available */
#define EKEYEXPIRED 127			/* Key has expired */
#define EKEYREVOKED 128			/* Key has been revoked */
#define EKEYREJECTED 129		/* Key was rejected by service */

/* for robust mutexes */
#define EOWNERDEAD  130			/* Owner died */
#define ENOTRECOVERABLE 131		/* State not recoverable */
#define ERFKILL     132			/* Operation not possible due to RF-kill */
#define EHWPOISON   133			/* Memory page has hardware error */
#define UNKNOWN_ERR 134

static const error syscall_errors[] = {
	{SUCCESS, "success"},
	{EPERM, "operation not permitted"},
	{ENOENT, "no such file or directory"},
	{ESRCH, "no such process"},
	{EINTR, "interrupted system call"},
	{EIO, "I/O error"},
	{ENXIO, "no such device or address"},
	{E2BIG, "argument list too long"},
	{ENOEXEC, "exec format error"},
	{EBADF, "bad file number"},
	{ECHILD, "no child processes"},
	{EAGAIN, "try again"},
	{ENOMEM, "out of memory"},
	{EACCES, "permission denied"},
	{EFAULT, "bad address"},
	{ENOTBLK, "block device required"},
	{EBUSY, "device or resource busy"},
	{EEXIST, "file exists"},
	{EXDEV, "cross-device link"},
	{ENODEV, "no such device"},
	{ENOTDIR, "not a directory"},
	{EISDIR, "is a directory"},
	{EINVAL, "invalid argument"},
	{ENFILE, "file table overflow"},
	{EMFILE, "too many open files"},
	{ENOTTY, "not a typewriter"},
	{ETXTBSY, "text file busy"},
	{EFBIG, "file too large"},
	{ENOSPC, "no space left on device"},
	{ESPIPE, "illegal seek"},
	{EROFS, "read-only file system"},
	{EMLINK, "too many links"},
	{EPIPE, "broken pipe"},
	{EDOM, "math argument out of domain of func"},
	{ERANGE, "math result not representable"},
	{EDEADLK, "resource deadlock would occur"},
	{ENAMETOOLONG, "file name too long"},
	{ENOLCK, "no record locks available"},
	{ENOSYS, "invalid system call number"},
	{ENOTEMPTY, "directory not empty"},
	{ELOOP, "too many symbolic links encountered"},
	{EAGAIN, "try again"},
	{ENOMSG, "no message of desired type"},
	{EIDRM, "identifier removed"},
	{ECHRNG, "channel number out of range"},
	{EL2NSYNC, "level 2 not synchronized"},
	{EL3HLT, "level 3 halted"},
	{EL3RST, "level 3 reset"},
	{ELNRNG, "link number out of range"},
	{EUNATCH, "protocol driver not attached"},
	{ENOCSI, "no CSI structure available"},
	{EL2HLT, "level 2 halted"},
	{EBADE, "invalid exchange"},
	{EBADR, "invalid request descriptor"},
	{EXFULL, "exchange full"},
	{ENOANO, "no anode"},
	{EBADRQC, "invalid request code"},
	{EBADSLT, "invalid slot"},
	{EDEADLOCK, "resource deadlock avoided"},
	{EBFONT, "bad font file format"},
	{ENOSTR, "device not a stream"},
	{ENODATA, "no data available"},
	{ETIME, "timer expired"},
	{ENOSR, "out of streams resources"},
	{ENONET, "machine is not on the network"},
	{ENOPKG, "package not installed"},
	{EREMOTE, "object is remote"},
	{ENOLINK, "link has been severed"},
	{EADV, "advertise error"},
	{ESRMNT, "srmount error"},
	{ECOMM, "communication error on send"},
	{EPROTO, "protocol error"},
	{EMULTIHOP, "multihop attempted"},
	{EDOTDOT, "RFS specific error"},
	{EBADMSG, "not a data message"},
	{EOVERFLOW, "value too large for defined data type"},
	{ENOTUNIQ, "name not unique on network"},
	{EBADFD, "file descriptor in bad state"},
	{EREMCHG, "remote address changed"},
	{ELIBACC, "can not access a needed shared library"},
	{ELIBBAD, "accessing a corrupted shared library"},
	{ELIBSCN, ".lib section in a.out corrupted"},
	{ELIBMAX, "attempting to link in too many shared libraries"},
	{ELIBEXEC, "cannot exec a shared library directly"},
	{EILSEQ, "illegal byte sequence"},
	{ERESTART, "interrupted system call should be restarted"},
	{ESTRPIPE, "streams pipe error"},
	{EUSERS, "too many users"},
	{ENOTSOCK, "socket operation on non-socket"},
	{EDESTADDRREQ, "destination address required"},
	{EMSGSIZE, "message too long"},
	{EPROTOTYPE, "protocol wrong type for socket"},
	{ENOPROTOOPT, "protocol not available"},
	{EPROTONOSUPPORT, "protocol not supported"},
	{ESOCKTNOSUPPORT, "socket type not supported"},
	{EOPNOTSUPP, "operation not supported on transport endpoint"},
	{EPFNOSUPPORT, "protocol family not supported"},
	{EAFNOSUPPORT, "address family not supported by protocol"},
	{EADDRINUSE, "address already in use"},
	{EADDRNOTAVAIL, "cannot assign requested address"},
	{ENETDOWN, "network is down"},
	{ENETUNREACH, "network is unreachable"},
	{ENETRESET, "network dropped connection because of reset"},
	{ECONNABORTED, "software caused connection abort"},
	{ECONNRESET, "connection reset by peer"},
	{ENOBUFS, "no buffer space available"},
	{EISCONN, "transport endpoint is already connected"},
	{ENOTCONN, "transport endpoint is not connected"},
	{ESHUTDOWN, "cannot send after transport endpoint shutdown"},
	{ETOOMANYREFS, "too many references: cannot splice"},
	{ETIMEDOUT, "connection timed out"},
	{ECONNREFUSED, "connection refused"},
	{EHOSTDOWN, "host is down"},
	{EHOSTUNREACH, "no route to host"},
	{EALREADY, "operation already in progress"},
	{EINPROGRESS, "operation now in progress"},
	{ESTALE, "stale file handle"},
	{EUCLEAN, "structure needs cleaning"},
	{ENOTNAM, "not a XENIX named type file"},
	{ENAVAIL, "no XENIX semaphores available"},
	{EISNAM, "is a named type file"},
	{EREMOTEIO, "remote I/O error"},
	{EDQUOT, "quota exceeded"},
	{ENOMEDIUM, "no medium found"},
	{EMEDIUMTYPE, "wrong medium type"},
	{ECANCELED, "operation canceled"},
	{ENOKEY, "required key not available"},
	{EKEYEXPIRED, "key has expired"},
	{EKEYREVOKED, "key has been revoked"},
	{EKEYREJECTED, "key was rejected by service"},
	{EOWNERDEAD, "owner died"},
	{ENOTRECOVERABLE, "state not recoverable"},
	{ERFKILL, "operation not possible due to RF-kill"},
	{EHWPOISON, "memory page has hardware error"},
	{UNKNOWN_ERR, "unknown error"}
};

#endif
