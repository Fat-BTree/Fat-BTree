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
  bit<8>  part_id;
}

header fbt_h {
  bit<32> key_hi;
  bit<32> key_lo;
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

#define META_VAR(b)                                                            \
  bit<32> key_##b##_hi;                                                        \
  bit<32> key_##b##_lo;                                                        \
  bit<48> idx_##b##;                                                           \
  bit<32> thresh_##b##_hi;                                                     \
  bit<32> thresh_##b##_lo;                                                     \
  bit<32> sub_##b##_hi;                                                        \
  bit<32> sub_##b##_lo;                                                        \
  bit<1> sgn_##b##_hi;                                                         \
  bit<1> sgn_##b##_lo;                                                         \
  bit<1> comp_0_##b##;                                                         \
  bit<1> comp_1_##b##;                                                         \
  bit<1> comp_2_##b##;                                                         \
  bit<1> comp_3_##b##;                                                         \
  bit<1> comp_4_##b##;                                                         \
  bit<1> comp_5_##b##;                                                         \
  bit<1> comp_6_##b##;                                                         \
  bit<1> comp_7_##b##;

struct my_ingress_metadata_t {
  META_VAR(0)
  META_VAR(1)
  META_VAR(2)
  META_VAR(3)

  // bit<32> result;
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
    default:
      accept;
    }
  }

#define META_INIT(b)                                                           \
  meta.idx_##b## = hdr.fbt.fbt_idx;                                            \
  meta.key_##b##_hi = hdr.fbt.key_hi;                                          \
  meta.key_##b##_lo = hdr.fbt.key_lo;

  state parse_fbt {
    pkt.extract(hdr.fbt);

    META_INIT(0)
    META_INIT(1)
    META_INIT(2)
    META_INIT(3)

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

#define SET_THRESH_TABLE_ACTION(a, b, c)                                       \
  action set_thresh_action_##a##_##b##(bit<32> thresh_hi, bit<32> thresh_lo) { \
    meta.thresh_##b##_hi = thresh_hi;                                          \
    meta.thresh_##b##_lo = thresh_lo;                                          \
    meta.sub_##b##_hi = thresh_hi - meta.key_##b##_hi;                         \
    meta.sub_##b##_lo = thresh_lo - meta.key_##b##_lo;                         \
    meta.sgn_##b##_hi = thresh_hi [31:31] ^ meta.key_##b##_hi [31:31];         \
    meta.sgn_##b##_lo = thresh_lo [31:31] ^ meta.key_##b##_lo [31:31];         \
  }                                                                            \
  @stage(##c##) table set_thresh_table_##a##_##b##{                            \
    actions = { set_thresh_action_##a##_##b##;                                 \
    NoAction;                                                                  \
  }                                                                            \
  key = { meta.idx_##b## : exact;                                              \
  }                                                                            \
  size = 10240;                                                                 \
  default_action = NoAction;                                                   \
  }

  SET_THRESH_TABLE_ACTION(0, 0, 0)
  SET_THRESH_TABLE_ACTION(0, 1, 0)
  SET_THRESH_TABLE_ACTION(0, 2, 0)
  SET_THRESH_TABLE_ACTION(0, 3, 0)

  SET_THRESH_TABLE_ACTION(1, 0, 1)
  SET_THRESH_TABLE_ACTION(1, 1, 1)
  SET_THRESH_TABLE_ACTION(1, 2, 1)
  SET_THRESH_TABLE_ACTION(1, 3, 1)

  SET_THRESH_TABLE_ACTION(2, 0, 2)
  SET_THRESH_TABLE_ACTION(2, 1, 2)
  SET_THRESH_TABLE_ACTION(2, 2, 2)
  SET_THRESH_TABLE_ACTION(2, 3, 2)

  SET_THRESH_TABLE_ACTION(3, 0, 3)
  SET_THRESH_TABLE_ACTION(3, 1, 3)
  SET_THRESH_TABLE_ACTION(3, 2, 3)
  SET_THRESH_TABLE_ACTION(3, 3, 3)

  SET_THRESH_TABLE_ACTION(4, 0, 6)
  SET_THRESH_TABLE_ACTION(4, 1, 6)
  SET_THRESH_TABLE_ACTION(4, 2, 6)
  SET_THRESH_TABLE_ACTION(4, 3, 6)

  SET_THRESH_TABLE_ACTION(5, 0, 7)
  SET_THRESH_TABLE_ACTION(5, 1, 7)
  SET_THRESH_TABLE_ACTION(5, 2, 7)
  SET_THRESH_TABLE_ACTION(5, 3, 7)

  SET_THRESH_TABLE_ACTION(6, 0, 8)
  SET_THRESH_TABLE_ACTION(6, 1, 8)
  SET_THRESH_TABLE_ACTION(6, 2, 8)
  SET_THRESH_TABLE_ACTION(6, 3, 8)

  SET_THRESH_TABLE_ACTION(7, 0, 9)
  SET_THRESH_TABLE_ACTION(7, 1, 9)
  SET_THRESH_TABLE_ACTION(7, 2, 9)
  SET_THRESH_TABLE_ACTION(7, 3, 9)

  /******************************************************************************/

  /******************************************************************************/

#define SET_COMP_TABLE_ACTION_HI(a, b, c)                                      \
  action set_comp_action_##a##_##b##_hi_s() {                                  \
    meta.comp_##a##_##b## = meta.sub_##b##_hi [31:31];                         \
  }                                                                            \
  action set_comp_action_##a##_##b##_hi_k() {                                  \
    meta.comp_##a##_##b## = meta.key_##b##_hi [31:31];                         \
  }                                                                            \
  @stage(##c##) table set_comp_table_##a##_##b##_hi {                          \
    actions = { set_comp_action_##a##_##b##_hi_s;                              \
    set_comp_action_##a##_##b##_hi_k;                                          \
    NoAction;                                                                  \
  }                                                                            \
  key = { meta.sgn_##b##_hi : exact;                                           \
  }                                                                            \
  size = 2;                                                                    \
  default_action = NoAction;                                                   \
  }

#define SET_COMP_TABLE_ACTION_LO(a, b, c)                                      \
  action set_comp_action_##a##_##b##_lo_s() {                                  \
    meta.comp_##a##_##b## = meta.sub_##b##_lo [31:31];                         \
  }                                                                            \
  action set_comp_action_##a##_##b##_lo_k() {                                  \
    meta.comp_##a##_##b## = meta.key_##b##_lo [31:31];                         \
  }                                                                            \
  @stage(##c##) table set_comp_table_##a##_##b##_lo {                          \
    actions = { set_comp_action_##a##_##b##_lo_s;                              \
    set_comp_action_##a##_##b##_lo_k;                                          \
    NoAction;                                                                  \
  }                                                                            \
  key = { meta.sgn_##b##_lo : exact;                                           \
  }                                                                            \
  size = 2;                                                                    \
  default_action = NoAction;                                                   \
  }

  SET_COMP_TABLE_ACTION_HI(0, 0, 1)
  SET_COMP_TABLE_ACTION_LO(0, 0, 1)
  SET_COMP_TABLE_ACTION_HI(0, 1, 1)
  SET_COMP_TABLE_ACTION_LO(0, 1, 1)
  SET_COMP_TABLE_ACTION_HI(0, 2, 1)
  SET_COMP_TABLE_ACTION_LO(0, 2, 1)
  SET_COMP_TABLE_ACTION_HI(0, 3, 1)
  SET_COMP_TABLE_ACTION_LO(0, 3, 1)

  SET_COMP_TABLE_ACTION_HI(1, 0, 2)
  SET_COMP_TABLE_ACTION_LO(1, 0, 2)
  SET_COMP_TABLE_ACTION_HI(1, 1, 2)
  SET_COMP_TABLE_ACTION_LO(1, 1, 2)
  SET_COMP_TABLE_ACTION_HI(1, 2, 2)
  SET_COMP_TABLE_ACTION_LO(1, 2, 2)
  SET_COMP_TABLE_ACTION_HI(1, 3, 2)
  SET_COMP_TABLE_ACTION_LO(1, 3, 2)

  SET_COMP_TABLE_ACTION_HI(2, 0, 3)
  SET_COMP_TABLE_ACTION_LO(2, 0, 3)
  SET_COMP_TABLE_ACTION_HI(2, 1, 3)
  SET_COMP_TABLE_ACTION_LO(2, 1, 3)
  SET_COMP_TABLE_ACTION_HI(2, 2, 3)
  SET_COMP_TABLE_ACTION_LO(2, 2, 3)
  SET_COMP_TABLE_ACTION_HI(2, 3, 3)
  SET_COMP_TABLE_ACTION_LO(2, 3, 3)

  SET_COMP_TABLE_ACTION_HI(3, 0, 4)
  SET_COMP_TABLE_ACTION_LO(3, 0, 4)
  SET_COMP_TABLE_ACTION_HI(3, 1, 4)
  SET_COMP_TABLE_ACTION_LO(3, 1, 4)
  SET_COMP_TABLE_ACTION_HI(3, 2, 4)
  SET_COMP_TABLE_ACTION_LO(3, 2, 4)
  SET_COMP_TABLE_ACTION_HI(3, 3, 4)
  SET_COMP_TABLE_ACTION_LO(3, 3, 4)

  SET_COMP_TABLE_ACTION_HI(4, 0, 7)
  SET_COMP_TABLE_ACTION_LO(4, 0, 7)
  SET_COMP_TABLE_ACTION_HI(4, 1, 7)
  SET_COMP_TABLE_ACTION_LO(4, 1, 7)
  SET_COMP_TABLE_ACTION_HI(4, 2, 7)
  SET_COMP_TABLE_ACTION_LO(4, 2, 7)
  SET_COMP_TABLE_ACTION_HI(4, 3, 7)
  SET_COMP_TABLE_ACTION_LO(4, 3, 7)

  SET_COMP_TABLE_ACTION_HI(5, 0, 8)
  SET_COMP_TABLE_ACTION_LO(5, 0, 8)
  SET_COMP_TABLE_ACTION_HI(5, 1, 8)
  SET_COMP_TABLE_ACTION_LO(5, 1, 8)
  SET_COMP_TABLE_ACTION_HI(5, 2, 8)
  SET_COMP_TABLE_ACTION_LO(5, 2, 8)
  SET_COMP_TABLE_ACTION_HI(5, 3, 8)
  SET_COMP_TABLE_ACTION_LO(5, 3, 8)

  SET_COMP_TABLE_ACTION_HI(6, 0, 9)
  SET_COMP_TABLE_ACTION_LO(6, 0, 9)
  SET_COMP_TABLE_ACTION_HI(6, 1, 9)
  SET_COMP_TABLE_ACTION_LO(6, 1, 9)
  SET_COMP_TABLE_ACTION_HI(6, 2, 9)
  SET_COMP_TABLE_ACTION_LO(6, 2, 9)
  SET_COMP_TABLE_ACTION_HI(6, 3, 9)
  SET_COMP_TABLE_ACTION_LO(6, 3, 9)

  SET_COMP_TABLE_ACTION_HI(7, 0, 10)
  SET_COMP_TABLE_ACTION_LO(7, 0, 10)
  SET_COMP_TABLE_ACTION_HI(7, 1, 10)
  SET_COMP_TABLE_ACTION_LO(7, 1, 10)
  SET_COMP_TABLE_ACTION_HI(7, 2, 10)
  SET_COMP_TABLE_ACTION_LO(7, 2, 10)
  SET_COMP_TABLE_ACTION_HI(7, 3, 10)
  SET_COMP_TABLE_ACTION_LO(7, 3, 10)

  /******************************************************************************/

#define RECORD_ACTION(a, b)                                                    \
  Register<bit<32>, bit<1>>(2) record_reg_##a##_##b##;                         \
  RegisterAction<bit<32>, bit<1>, bit<1>>(record_reg_##a##_##b##)              \
      record_##a##_##b## = {                                                   \
          void apply(inout bit<32> record_data,                                \
                     out bit<1> result){record_data = record_data + 1;         \
  }                                                                            \
  }                                                                            \
  ;



  /******************************************************************************/
  action set_result_action(bit<48> result) { 
      hdr.fbt.fbt_idx = result;
      meta.idx_0 = result;
      meta.idx_1 = result;
      meta.idx_2 = result;
      meta.idx_3 = result;
  }

  @stage(5) table set_result_table {
    actions = { set_result_action;
    NoAction;
  }
  key = { 
  hdr.fbt.fbt_idx : exact;
  meta.comp_0_0 : exact;
  meta.comp_0_1 : exact;
  meta.comp_0_2 : exact;
  meta.comp_0_3 : exact;
  meta.comp_1_0 : exact;
  meta.comp_1_1 : exact;
  meta.comp_1_2 : exact;
  meta.comp_1_3 : exact;
  meta.comp_2_0 : exact;
  meta.comp_2_1 : exact;
  meta.comp_2_2 : exact;
  meta.comp_2_3 : exact;
  meta.comp_3_0 : exact;
  meta.comp_3_1 : exact;
  meta.comp_3_2 : exact;
  meta.comp_3_3 : exact;
}

  size = 65536;
  default_action = NoAction;
}

  action set_result_action_2(bit<48> result) { 
      hdr.fbt.fbt_idx = result;
  }
  @stage(11) table set_result_table_2 {
    actions = { set_result_action_2;
    NoAction;
  }
  key = { 
  hdr.fbt.fbt_idx : exact;
  meta.comp_4_0 : exact;
  meta.comp_4_1 : exact;
  meta.comp_4_2 : exact;
  meta.comp_4_3 : exact;
  meta.comp_5_0 : exact;
  meta.comp_5_1 : exact;
  meta.comp_5_2 : exact;
  meta.comp_5_3 : exact;
  meta.comp_6_0 : exact;
  meta.comp_6_1 : exact;
  meta.comp_6_2 : exact;
  meta.comp_6_3 : exact;
  meta.comp_7_0 : exact;
  meta.comp_7_1 : exact;
  meta.comp_7_2 : exact;
  meta.comp_7_3 : exact;
}

  size = 65536;
  default_action = NoAction;
}



/******************************************************************************/

  action set_port_action(bit<9> port) {
  ig_tm_md.ucast_egress_port = port;
  }
  @stage(0) table set_port_table {
    actions = { set_port_action;
    NoAction;
  }
  key = { 
        hdr.msg.part_id : exact;
        ig_intr_md.ingress_port : exact; 
  }
  default_action = NoAction;
}

/******************************************************************************/

apply {
  set_port_table.apply();
  if (hdr.fbt.isValid()) {

#define APPLY_TABLE(a)                                                         \
  set_thresh_table_##a##_0.apply();                                            \
  set_thresh_table_##a##_1.apply();                                            \
  set_thresh_table_##a##_2.apply();                                            \
  set_thresh_table_##a##_3.apply();                                            \
  if (meta.sub_0_hi == 0) {                                                    \
    set_comp_table_##a##_0_lo.apply();                                         \
  } else {                                                                     \
    set_comp_table_##a##_0_hi.apply();                                         \
  }                                                                            \
  if (meta.sub_1_hi == 0) {                                                    \
    set_comp_table_##a##_1_lo.apply();                                         \
  } else {                                                                     \
    set_comp_table_##a##_1_hi.apply();                                         \
  }                                                                            \
  if (meta.sub_2_hi == 0) {                                                    \
    set_comp_table_##a##_2_lo.apply();                                         \
  } else {                                                                     \
    set_comp_table_##a##_2_hi.apply();                                         \
  }                                                                            \
  if (meta.sub_3_hi == 0) {                                                    \
    set_comp_table_##a##_3_lo.apply();                                         \
  } else {                                                                     \
    set_comp_table_##a##_3_hi.apply();                                         \
  }

    APPLY_TABLE(0);
    APPLY_TABLE(1);
    APPLY_TABLE(2);
    APPLY_TABLE(3);

    set_result_table.apply();

    APPLY_TABLE(4);
    APPLY_TABLE(5);
    APPLY_TABLE(6);
    APPLY_TABLE(7);

    set_result_table_2.apply();
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
  apply { 
    pkt.emit(hdr); 
  }
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
