#ifndef _ROUTE_ITERATOR_H_
#define _ROUTE_ITERATOR_H_

#include "sfsmisc.h"
#include "crypt.h"
#include "refcnt.h"

class vnode;
typedef callback<void, bool>::ptr cbhop_t;

class route_iterator {
 protected:
  ptr<vnode> v;
  chordID x;
  route search_path;
  chordstat r;
  bool done;
  cbhop_t cb;
  ptr<bool> deleted;

  bool do_upcall;
  int uc_procno;
  ptr<void> uc_args;
  rpc_program prog;

  bool stop;
  bool last_hop;

 public:
  route_iterator (ptr<vnode> vi, chordID xi) :
    v (vi), x (xi), r (CHORD_OK), done (false), 
    deleted (New refcounted<bool> (false)),
    do_upcall (false), stop (false), last_hop (false) {};
  route_iterator (ptr<vnode> vi, chordID xi,
		  rpc_program uc_prog,
		  int uc_procno,
		  ptr<void> uc_args) :
    v (vi), x (xi), r (CHORD_OK), done (false), 
    deleted (New refcounted<bool> (false)),
    do_upcall (true),
    uc_procno (uc_procno), uc_args (uc_args), prog (prog),
    stop (false), last_hop (false) {};

  virtual ~route_iterator () {};

  chordID last_node () { return search_path.back (); };
  chordID key () { return x; };
  route path () { return search_path; };
  chordstat status () { return r; };

  virtual void print ();
  virtual void first_hop (cbhop_t cb, bool ucs = false) = 0;
  virtual void first_hop (cbhop_t cb, chordID guess) = 0;
  virtual void next_hop () {};
  virtual void send (chordID guess) = 0;
  virtual void send (bool ucs) = 0;

  static char * marshall_upcall_args (rpc_program *prog, 
				      int uc_procno,
				      ptr<void> uc_args,
				      int *upcall_args_len);
  
  static bool unmarshall_upcall_res (rpc_program *prog, 
				     int uc_procno, 
				     void *upcall_res,
				     int upcall_res_len,
				     void *dest);

};

class route_chord : public route_iterator {
  void make_hop (chordID &n);
  void make_hop_cb (ptr<bool> del, chord_testandfindres *res, clnt_stat err);
  void make_route_done_cb (chordID s, bool ok, chordstat status);
  void make_hop_done_cb (chordID s, bool ok, chordstat status);
  void send_hop_cb (bool done);

 public:
  route_chord (ptr<vnode> vi, chordID xi);
  route_chord (ptr<vnode> vi, chordID xi,
	       rpc_program uc_prog,
	       int uc_procno,
	       ptr<void> uc_args);

  ~route_chord () {*deleted = true;};
  virtual void first_hop (cbhop_t cb, bool ucs = false);
  virtual void first_hop (cbhop_t cb, chordID guess);
  void send (chordID guess);
  void send (bool ucs = false);

  void next_hop ();
};

class route_debruijn : public route_iterator {
  int hops;
  route virtual_path;
  void make_hop (chordID &n, chordID &x, chordID &d);
  void make_hop_cb (chord_debruijnres *res, clnt_stat err);
  void make_route_done_cb (chordID s, bool ok, chordstat status);
  void make_hop_done_cb (chordID d, chordID s, bool ok, chordstat status);
  void send_hop_cb (bool done);

 public:
  route_debruijn (ptr<vnode> vi, chordID xi);
  route_debruijn (ptr<vnode> vi, chordID xi,
		  rpc_program uc_prog,
		  int uc_procno,
		  ptr<void> uc_args);

  void send (chordID guess);
  void send (bool ucs);
  virtual void first_hop (cbhop_t cb, bool ucs = false);
  virtual void first_hop (cbhop_t cb, chordID guess);
  void print ();
  void next_hop ();
};

class route_factory {
protected:
  ptr<vnode> vi;
public:
  route_factory (ptr<vnode> vi) : vi (vi) {};
  route_factory () {};
  
  void setvnode (ptr<vnode> v) { vi = v; };
  virtual ~route_factory () {};
  virtual ptr<route_iterator> produce_iterator (chordID xi) = 0;
  virtual ptr<route_iterator> produce_iterator (chordID xi,
						rpc_program uc_prog,
						int uc_procno,
						ptr<void> uc_args) = 0;
  virtual route_iterator* produce_iterator_ptr (chordID xi) = 0;
  virtual route_iterator* produce_iterator_ptr (chordID xi,
						rpc_program uc_prog,
						int uc_procno,
						ptr<void> uc_args) = 0;
  ptr<vnode> get_vnode () {return vi;};
  void get_node (chord_node *n);
};

class debruijn_route_factory : public route_factory {
public:
  debruijn_route_factory (ptr<vnode> vi) : route_factory (vi) {};
  debruijn_route_factory () {};
  ptr<route_iterator> produce_iterator (chordID xi);
  ptr<route_iterator> produce_iterator (chordID xi,
					rpc_program uc_prog,
					int uc_procno,
					ptr<void> uc_args);
  virtual route_iterator* produce_iterator_ptr (chordID xi);
  virtual route_iterator* produce_iterator_ptr (chordID xi,
					rpc_program uc_prog,
					int uc_procno,
					ptr<void> uc_args);
};

class chord_route_factory : public route_factory {
public:
  chord_route_factory (ptr<vnode> vi) : route_factory (vi) {};
  chord_route_factory () {};
  ptr<route_iterator> produce_iterator (chordID xi);
  ptr<route_iterator> produce_iterator (chordID xi,
					rpc_program uc_prog,
					int uc_procno,
					ptr<void> uc_args);
  virtual route_iterator* produce_iterator_ptr (chordID xi);
  virtual route_iterator* produce_iterator_ptr (chordID xi,
					rpc_program uc_prog,
					int uc_procno,
					ptr<void> uc_args);
};
#endif /* _ROUTE_ITERATOR_H_ */
