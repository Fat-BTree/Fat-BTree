/* -*- P4_16 -*- */
#include <core.p4>
#include <tna.p4>

/*************************************************************************
 ************* C O N S T A N T S A N D T Y P E S *******************
 *************************************************************************/
enum bit<16> ether_type_t { IPV4 = 0x0800 }

enum bit<8> ip_proto_t{UDP = 17}

type bit<48>
    mac_addr_t;

/*************************************************************************
 *********************** H E A D E R S *********************************
 *************************************************************************/
/* Define all the headers the program will recognize */
/* The actual sets of headers processed by each gress can differ */

/* Standard ethernet header */
header ethernet_h {
  mac_addr_t dst_addr;
  mac_addr_t src_addr;
  ether_type_t ether_type;
}

header ipv4_h {
  bit<4> version;
  bit<4> ihl;
  bit<7> diffserv;
  bit<1> res;
  bit<16> total_len;
  bit<16> identification;
  bit<3> flags;
  bit<13> frag_offset;
  bit<8> ttl;
  bit<8> protocol;
  bit<16> hdr_checksum;
  bit<32> src_addr;
  bit<32> dst_addr;
}

header udp_h {
  bit<16> src_port;
  bit<16> dst_port;
  bit<16> len;
  bit<16> checksum;
}
header msg_h {
  bit<32> msg_type;
  bit<8> part_id;
}
header fbt_h {
  bit<16> key_1;
  bit<16> key_2;
  bit<16> key_3;
  bit<16> key_4;
  // bit<16> padding;
  bit<48> fbt_idx;
}

/*************************************************************************
 ************** I N G R E S S P R O C E S S I N G *******************
 *************************************************************************/

/*********************** H E A D E R S ************************/

struct my_ingress_headers_t {
  ethernet_h ethernet;
  ipv4_h ipv4;
  udp_h udp;
  msg_h msg;
  fbt_h fbt;
}

/****** G L O B A L I N G R E S S M E T A D A T A *********/

struct my_ingress_metadata_t {
  bit<16> key_1;
  bit<16> key_2;
  bit<16> key_3;
  bit<16> key_4;
  bit<48> idx;
  bit<32> result;
  bit<1>  nexttable;
}
/*********************** P A R S E R **************************/

parser
IngressParser(packet_in pkt,
              /* User */
              out my_ingress_headers_t hdr, out my_ingress_metadata_t meta,
              /* Intrinsic */
              out ingress_intrinsic_metadata_t ig_intr_md) {
  state start {
    pkt.extract(ig_intr_md);
    pkt.advance(PORT_METADATA_SIZE);
    transition parse_ethernet;
  }

  state parse_ethernet {
    pkt.extract(hdr.ethernet);
    transition select((bit<16>)hdr.ethernet.ether_type) {
      (bit<16>)ether_type_t.IPV4 : parse_ipv4;
    default:
      accept;
    }
  }

  state parse_ipv4 {
    pkt.extract(hdr.ipv4);
    transition select(hdr.ipv4.protocol) {
      17 : parse_udp;
    default:
      accept;
    }
  }

  state parse_udp {
    pkt.extract(hdr.udp);
    pkt.extract(hdr.msg);
	transition select(hdr.msg.msg_type) {
      0x0527 : parse_fbt;
      default : accept;
    }
  }
  state parse_fbt {
    pkt.extract(hdr.fbt);
	meta.key_1 = hdr.fbt.key_1;
	meta.key_2 = hdr.fbt.key_2;
	meta.key_3 = hdr.fbt.key_3;
	meta.key_4 = hdr.fbt.key_4;
    transition accept;
  }
}

control Ingress(/* User */
                inout my_ingress_headers_t hdr,
                inout my_ingress_metadata_t meta,
                /* Intrinsic */
                in ingress_intrinsic_metadata_t ig_intr_md,
                in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
                inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
                inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

  /******************************************************************************/


  action set_result_action_1(bit<32> result, bit<1> nexttable) { 
    meta.result = result; 
    meta.nexttable = nexttable;
  }
  action set_result_action_2(bit<32> result, bit<1> nexttable) { 
    meta.result = result; 
    meta.nexttable = nexttable;
  }
  action set_result_action_3(bit<32> result, bit<1> nexttable) { 
    meta.result = result; 
    meta.nexttable = nexttable;
  }
  action set_result_action_4(bit<32> result) { 
    meta.result = result; 
  }           
table set_result_table_1 {
  actions = {
    set_result_action_1;
    NoAction;
  }
  key = {
    meta.key_1 : range;
  }
  default_action = NoAction;
  size = 4096;
}
table set_result_table_2 {
  actions = {
    set_result_action_2;
    NoAction;
  }
  key = {
    meta.result : exact;
    meta.key_2 : range;
  }
  default_action = NoAction;
  size = 4096;
}
table set_result_table_3 {
  actions = {
    set_result_action_3;
    NoAction;
  }
  key = {
    meta.result : exact;
    meta.key_3 : range;
  }
  default_action = NoAction;
  size = 4096;
}
table set_result_table_4 {
  actions = {
    set_result_action_4;
    NoAction;
  }
  key = {
    meta.result : exact;
    meta.key_4 : range;
  }
  default_action = NoAction;
  size = 4096;
}
action set_port_action(bit<9> port){
  ig_tm_md.ucast_egress_port = port;
}
table set_port_table {
  actions = {
    set_port_action;
    NoAction;
  }
  key = {
    hdr.msg.part_id : exact;
    ig_intr_md.ingress_port: exact;
  }
  default_action = NoAction;
}
action set_addr(bit<48> addr) {
  hdr.fbt.fbt_idx = addr;
}
table set_addr_table {
  actions = {
    set_addr;
  }
  key = {
    meta.result: exact;
  }
  size = 4096;
}
/******************************************************************************/

apply {
  set_port_table.apply();
  if (hdr.fbt.isValid()) {
	meta.key_1 = hdr.fbt.key_1;
	meta.key_2 = hdr.fbt.key_2;
	meta.key_3 = hdr.fbt.key_3;
	meta.key_4 = hdr.fbt.key_4;
    set_result_table_1.apply();
    if(meta.nexttable == 1){
      set_result_table_2.apply();
      if(meta.nexttable == 1){
        set_result_table_3.apply();
        if(meta.nexttable == 1){
          set_result_table_4.apply();
        }
      }
    }
    set_addr_table.apply();
  }
}
}

/**************************D E P A R S E R*******************/

control
IngressDeparser(packet_out pkt,
                /* User */
                inout my_ingress_headers_t hdr, in my_ingress_metadata_t meta,
                /* Intrinsic */
                in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
  apply { pkt.emit(hdr); }
}
/*************************************************************************
 **************** E G R E S S P R O C E S S I N G *******************
 *************************************************************************/

/*********************** H E A D E R S ************************/

struct my_egress_headers_t {
}

/******** G L O B A L E G R E S S M E T A D A T A *********/

struct my_egress_metadata_t {
}

/*********************** P A R S E R **************************/

parser
EgressParser(packet_in pkt,
             /* User */
             out my_egress_headers_t hdr, out my_egress_metadata_t meta,
             /* Intrinsic */
             out egress_intrinsic_metadata_t eg_intr_md) {
  /* This is a mandatory state, required by Tofino Architecture */
  state start {
    pkt.extract(eg_intr_md);
    transition accept;
  }
}

/***************** M A T C H - A C T I O N *********************/

control Egress(
    /* User */
    inout my_egress_headers_t hdr, inout my_egress_metadata_t meta,
    /* Intrinsic */
    in egress_intrinsic_metadata_t eg_intr_md,
    in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
    inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
    inout egress_intrinsic_metadata_for_output_port_t eg_oport_md) {
  apply {}
}

/********************* D E P A R S E R ************************/

control
EgressDeparser(packet_out pkt,
               /* User */
               inout my_egress_headers_t hdr, in my_egress_metadata_t meta,
               /* Intrinsic */
               in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
  apply { pkt.emit(hdr); }
}

/************ F I N A L P A C K A G E ******************************/
Pipeline(IngressParser(), Ingress(), IngressDeparser(), EgressParser(),
         Egress(), EgressDeparser()) pipe;
Switch(pipe) main;
