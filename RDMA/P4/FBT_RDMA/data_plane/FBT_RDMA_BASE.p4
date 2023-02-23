/* -*- P4_16 -*- */
#include <core.p4>
#include <tna.p4>

/*************************************************************************
 ************* C O N S T A N T S A N D T Y P E S *******************
 *************************************************************************/
enum bit<16> ether_type_t { IPV4 = 0x0800 }

enum bit<8> ip_proto_t { UDP = 17 }

type bit<48> mac_addr_t;

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

header fbt_h {
  bit<32> key_hi;
  bit<32> key_lo;
  bit<48> fbt_idx;
}

header ib_bth_t {
    bit<8>  opcode;
    bit<1>  event;
    bit<1>  migReq;
    bit<2>  padCount;
    bit<4>  version;
    bit<16> pkey;
    bit<8>  resv8a;
    bit<24> dstQP;
    bit<1>  ackReq;
    bit<7>  resv7b;
    bit<24> psn;
}

header ib_reth_t {
    bit<64> va;
    bit<32> rkey;
    bit<32> length;
}

header ib_icrc_t {
    bit<32> crc;
}

#define META_VAR_1(b)                                                          \
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

#define META_INIT_1(b)                                                         \
  meta.idx_##b## = hdr.fbt.fbt_idx;                                            \
  meta.key_##b##_hi = hdr.fbt.key_hi;                                          \
  meta.key_##b##_lo = hdr.fbt.key_lo;

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
  size = 4096;                                                                \
  default_action = NoAction;                                                   \
  }

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

#define RECORD_ACTION(a, b, c)                                                 \
  Register<bit<##c##>, bit<1>>(2) record_reg_##a##;                            \
  RegisterAction<bit<##c##>, bit<1>, bit<1>>(record_reg_##a##)                 \
      record_##a## = {                                                         \
          void apply(inout bit<##c##> record_data,                             \
                     out bit<1> result){record_data = ##b##;                   \
  }                                                                            \
  }                                                                            \
  ;

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

/*************************************************************************
 ************** I N G R E S S P R O C E S S I N G *******************
 *************************************************************************/

/*********************** H E A D E R S ************************/

struct my_ingress1_headers_t {
    ethernet_h ethernet;
    ipv4_h     ipv4;
    udp_h      udp;
    ib_bth_t   ib_bth;
    ib_reth_t  ib_reth;
    ib_icrc_t  ib_icrc;
    fbt_h      fbt;
}

/****** G L O B A L I N G R E S S M E T A D A T A *********/

struct my_ingress1_metadata_t {
  META_VAR_1(0)
  META_VAR_1(1)
  META_VAR_1(2)
  META_VAR_1(3)
}

/*********************** P A R S E R **************************/

parser
IngressParser1(packet_in pkt,
              /* User */
              out my_ingress1_headers_t hdr, out my_ingress1_metadata_t meta,
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
        default: accept;
    }
  }

  state parse_ipv4 {
    pkt.extract(hdr.ipv4);
    transition select(hdr.ipv4.protocol) {
        17 : parse_udp;
        default: accept;
    }
  }

  state parse_udp {
    pkt.extract(hdr.udp);
    transition select(hdr.udp.dst_port) {
        4791: parse_ib_bth_1;
        default: accept;
    }
  }

  state parse_ib_bth_1 {
    pkt.extract(hdr.ib_bth);
    transition select(hdr.ib_bth.opcode) {
        12: parse_ib_reth_1;
        default: accept;
    }
  }

  state parse_ib_reth_1 {
      pkt.extract(hdr.ib_reth);
      pkt.extract(hdr.ib_icrc);

      transition select(hdr.ib_reth.rkey) {
          0: parse_fbt_1;
          default: accept;
      }
  }

  state parse_fbt_1 {
      pkt.extract(hdr.fbt);

      META_INIT_1(0)
      META_INIT_1(1)
      META_INIT_1(2)
      META_INIT_1(3)

      transition accept;
  }
}

control Ingress1(/* User */
                inout my_ingress1_headers_t hdr,
                inout my_ingress1_metadata_t meta,
                /* Intrinsic */
                in ingress_intrinsic_metadata_t ig_intr_md,
                in ingress_intrinsic_metadata_from_parser_t ig_prsr_md,
                inout ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md,
                inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {

  /******************************************************************************/

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

  action set_result_action_1(bit<48> result) { 
      hdr.fbt.fbt_idx = result;
      meta.idx_0 = result;
      meta.idx_1 = result;
      meta.idx_2 = result;
      meta.idx_3 = result;
  }

  @stage(5) table set_result_table_1 {
      actions = { 
          set_result_action_1;
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
      actions = {
          set_result_action_2;
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
      actions = {
          set_port_action;
          NoAction;
      }
      key = {
          ig_intr_md.ingress_port : exact; 
      }
      default_action = NoAction;
  }


    apply {
      if (hdr.fbt.isValid()) {

        APPLY_TABLE(0);
        APPLY_TABLE(1);
        APPLY_TABLE(2);
        APPLY_TABLE(3);

        set_result_table_1.apply();

        APPLY_TABLE(4);
        APPLY_TABLE(5);
        APPLY_TABLE(6);
        APPLY_TABLE(7);

        set_result_table_2.apply();
      }

      set_port_table.apply();
    }
}

/**************************D E P A R S E R*******************/

control
IngressDeparser1(packet_out pkt,
        /* User */
        inout my_ingress1_headers_t hdr, in my_ingress1_metadata_t meta,
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

struct my_egress1_headers_t {
}

/******** G L O B A L E G R E S S M E T A D A T A *********/

struct my_egress1_metadata_t {
}

/*********************** P A R S E R **************************/

parser
EgressParser1(packet_in pkt,
        /* User */
        out my_egress1_headers_t hdr, out my_egress1_metadata_t meta,
        /* Intrinsic */
        out egress_intrinsic_metadata_t eg_intr_md) {
    /* This is a mandatory state, required by Tofino Architecture */
    state start {
        pkt.extract(eg_intr_md);
        transition accept;
    }
}

/***************** M A T C H - A C T I O N *********************/

control Egress1(
        /* User */
        inout my_egress1_headers_t hdr, inout my_egress1_metadata_t meta,
        /* Intrinsic */
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_oport_md) {
    apply {}
}

/********************* D E P A R S E R ************************/

control
EgressDeparser1(packet_out pkt,
        /* User */
        inout my_egress1_headers_t hdr, in my_egress1_metadata_t meta,
        /* Intrinsic */
        in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {

    apply { pkt.emit(hdr); }
}

#define META_VAR_0(b)                                                          \
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
  bit<1> comp_3_##b##;

#define META_INIT_0(b)                                                         \
  meta.idx_##b## = 0;                                                          \
  meta.key_##b##_hi = hdr.ib_reth.va[63:32];                                   \
  meta.key_##b##_lo = hdr.ib_reth.va[31:0];

/*************************************************************************
 ************** I N G R E S S P R O C E S S I N G *******************
 *************************************************************************/

/*********************** H E A D E R S ************************/

struct my_ingress0_headers_t {
    ethernet_h ethernet;
    ipv4_h     ipv4;
    udp_h      udp;
    ib_bth_t   ib_bth;
    ib_reth_t  ib_reth;
    ib_icrc_t  ib_icrc;
    fbt_h      fbt;
}

/****** G L O B A L I N G R E S S M E T A D A T A *********/

struct my_ingress0_metadata_t {
  bit<16> key_1;
  bit<16> key_2;
  bit<16> key_3;
  bit<16> key_4;

  bit<32> result;
  bit<1>  nexttable;

  META_VAR_0(0)
  META_VAR_0(1)
  META_VAR_0(2)
  META_VAR_0(3)
}

/*********************** P A R S E R **************************/

parser
IngressParser0(packet_in pkt,
              /* User */
              out my_ingress0_headers_t hdr, out my_ingress0_metadata_t meta,
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
        default: accept;
    }
  }

  state parse_ipv4 {
    pkt.extract(hdr.ipv4);
    transition select(hdr.ipv4.protocol) {
        17 : parse_udp;
        default: accept;
    }
  }

  state parse_udp {
    pkt.extract(hdr.udp);
    transition select(hdr.udp.dst_port) {
        4791: parse_ib_bth_0;
        default: accept;
    }
  }

  state parse_ib_bth_0 {
    pkt.extract(hdr.ib_bth);
    transition select(hdr.ib_bth.opcode) {
        12: parse_ib_reth_0;
        default: accept;
    }
  }

  state parse_ib_reth_0 {
      pkt.extract(hdr.ib_reth);
      pkt.extract(hdr.ib_icrc);

	  meta.key_1 = hdr.ib_reth.va[63:48];
	  meta.key_2 = hdr.ib_reth.va[47:32];
	  meta.key_3 = hdr.ib_reth.va[31:16];
	  meta.key_4 = hdr.ib_reth.va[15:0];

      META_INIT_0(0)
      META_INIT_0(1)
      META_INIT_0(2)
      META_INIT_0(3)

      transition select(hdr.ib_reth.rkey) {
          0: parse_fbt_0;
          default: accept;
      }
  }

  state parse_fbt_0 {
      hdr.fbt.setValid();

      transition accept;
  }
}

control Ingress0(/* User */
                inout my_ingress0_headers_t hdr,
                inout my_ingress0_metadata_t meta,
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

    @stage(1) table set_result_table_1 {
        actions = {
            set_result_action_1;
            NoAction;
        }
        key = {
            meta.key_1 : range;
        }
        default_action = NoAction;
        size = 2048;
    }

    @stage(2) table set_result_table_2 {
        actions = {
            set_result_action_2;
            NoAction;
        }
        key = {
            meta.result : exact;
            meta.key_2 : range;
        }
        default_action = NoAction;
        size = 2048;
    }

    @stage(3) table set_result_table_3 {
        actions = {
            set_result_action_3;
            NoAction;
        }
        key = {
            meta.result : exact;
            meta.key_3 : range;
        }
        default_action = NoAction;
        size = 2048;
    }

    @stage(4) table set_result_table_4 {
        actions = {
            set_result_action_4;
            NoAction;
        }
        key = {
            meta.result : exact;
            meta.key_4 : range;
        }
        default_action = NoAction;
        size = 2048;
    }

    action set_addr(bit<48> result) {
        hdr.fbt.fbt_idx = result;
        meta.idx_0 = result;
        meta.idx_1 = result;
        meta.idx_2 = result;
        meta.idx_3 = result;
    }

    @stage(5) table set_addr_table {
        actions = {
            set_addr;
        }
        key = {
            meta.result: exact;
        }
        size = 2048;
    }

  /******************************************************************************/

  SET_THRESH_TABLE_ACTION(0, 0, 6)
  SET_THRESH_TABLE_ACTION(0, 1, 6)
  SET_THRESH_TABLE_ACTION(0, 2, 6)
  SET_THRESH_TABLE_ACTION(0, 3, 6)

  SET_THRESH_TABLE_ACTION(1, 0, 7)
  SET_THRESH_TABLE_ACTION(1, 1, 7)
  SET_THRESH_TABLE_ACTION(1, 2, 7)
  SET_THRESH_TABLE_ACTION(1, 3, 7)

  SET_THRESH_TABLE_ACTION(2, 0, 8)
  SET_THRESH_TABLE_ACTION(2, 1, 8)
  SET_THRESH_TABLE_ACTION(2, 2, 8)
  SET_THRESH_TABLE_ACTION(2, 3, 8)

  SET_THRESH_TABLE_ACTION(3, 0, 9)
  SET_THRESH_TABLE_ACTION(3, 1, 9)
  SET_THRESH_TABLE_ACTION(3, 2, 9)
  SET_THRESH_TABLE_ACTION(3, 3, 9)

  /******************************************************************************/

  SET_COMP_TABLE_ACTION_HI(0, 0, 7)
  SET_COMP_TABLE_ACTION_LO(0, 0, 7)
  SET_COMP_TABLE_ACTION_HI(0, 1, 7)
  SET_COMP_TABLE_ACTION_LO(0, 1, 7)
  SET_COMP_TABLE_ACTION_HI(0, 2, 7)
  SET_COMP_TABLE_ACTION_LO(0, 2, 7)
  SET_COMP_TABLE_ACTION_HI(0, 3, 7)
  SET_COMP_TABLE_ACTION_LO(0, 3, 7)

  SET_COMP_TABLE_ACTION_HI(1, 0, 8)
  SET_COMP_TABLE_ACTION_LO(1, 0, 8)
  SET_COMP_TABLE_ACTION_HI(1, 1, 8)
  SET_COMP_TABLE_ACTION_LO(1, 1, 8)
  SET_COMP_TABLE_ACTION_HI(1, 2, 8)
  SET_COMP_TABLE_ACTION_LO(1, 2, 8)
  SET_COMP_TABLE_ACTION_HI(1, 3, 8)
  SET_COMP_TABLE_ACTION_LO(1, 3, 8)

  SET_COMP_TABLE_ACTION_HI(2, 0, 9)
  SET_COMP_TABLE_ACTION_LO(2, 0, 9)
  SET_COMP_TABLE_ACTION_HI(2, 1, 9)
  SET_COMP_TABLE_ACTION_LO(2, 1, 9)
  SET_COMP_TABLE_ACTION_HI(2, 2, 9)
  SET_COMP_TABLE_ACTION_LO(2, 2, 9)
  SET_COMP_TABLE_ACTION_HI(2, 3, 9)
  SET_COMP_TABLE_ACTION_LO(2, 3, 9)

  SET_COMP_TABLE_ACTION_HI(3, 0, 10)
  SET_COMP_TABLE_ACTION_LO(3, 0, 10)
  SET_COMP_TABLE_ACTION_HI(3, 1, 10)
  SET_COMP_TABLE_ACTION_LO(3, 1, 10)
  SET_COMP_TABLE_ACTION_HI(3, 2, 10)
  SET_COMP_TABLE_ACTION_LO(3, 2, 10)
  SET_COMP_TABLE_ACTION_HI(3, 3, 10)
  SET_COMP_TABLE_ACTION_LO(3, 3, 10)

  /******************************************************************************/

  action set_result_action_0(bit<48> result) { 
      hdr.fbt.fbt_idx = result;
  }

  @stage(11) table set_result_table_0 {
      actions = { 
          set_result_action_0;
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

/******************************************************************************/

    action set_port_action(bit<9> port) {
        ig_tm_md.ucast_egress_port = port;
    }

    @stage(0) table set_port_table {
        actions = {
            set_port_action;
            NoAction;
        }
        key = {
            ig_intr_md.ingress_port : exact; 
        }
        default_action = NoAction;
    }

    action set_hdr_fbt_action() {
      hdr.fbt.key_hi = hdr.ib_reth.va[63:32];
      hdr.fbt.key_lo = hdr.ib_reth.va[31:0];
      hdr.fbt.fbt_idx = 0;
    }

    @stage(0) table set_hdr_fbt_table {
        actions = {
            set_hdr_fbt_action;
        }
        default_action = set_hdr_fbt_action;
    }


    apply {
        if (hdr.fbt.isValid()) {
            set_hdr_fbt_table.apply();

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

            APPLY_TABLE(0);
            APPLY_TABLE(1);
            APPLY_TABLE(2);
            APPLY_TABLE(3);

            set_result_table_0.apply();
        }

        set_port_table.apply();
    }
}

/**************************D E P A R S E R*******************/

control
IngressDeparser0(packet_out pkt,
        /* User */
        inout my_ingress0_headers_t hdr, in my_ingress0_metadata_t meta,
        /* Intrinsic */
        in ingress_intrinsic_metadata_for_deparser_t ig_dprsr_md) {
    apply { pkt.emit(hdr); }
}

/*************************************************************************
 **************** E G R E S S P R O C E S S I N G *******************
 *************************************************************************/

/*********************** H E A D E R S ************************/

struct my_egress0_headers_t {
    ethernet_h ethernet;
    ipv4_h     ipv4;
    udp_h      udp;
    ib_bth_t   ib_bth;
    ib_reth_t  ib_reth;
    ib_icrc_t  ib_icrc;
    fbt_h      fbt;
}

/******** G L O B A L E G R E S S M E T A D A T A *********/

struct my_egress0_metadata_t {
    bit<32> crc32;
    bit<32> crc32revs;

    bit<64> remote_addr;
    bit<32> remote_stag;

    bit<64> resv64a;
    bit<8>  resv8b;
    bit<8>  resv8c;
    bit<16> resv16d;
    bit<16> resv16e;
    bit<8>  resv8f;
}

/*********************** P A R S E R **************************/

parser
EgressParser0(packet_in pkt,
        /* User */
        out my_egress0_headers_t hdr, out my_egress0_metadata_t meta,
        /* Intrinsic */
        out egress_intrinsic_metadata_t eg_intr_md) {
    /* This is a mandatory state, required by Tofino Architecture */
  state start {
      pkt.extract(eg_intr_md);
      transition parse_ethernet;
  }

  state parse_ethernet {
    pkt.extract(hdr.ethernet);
    transition select((bit<16>)hdr.ethernet.ether_type) {
        (bit<16>)ether_type_t.IPV4 : parse_ipv4;
        default: accept;
    }
  }

  state parse_ipv4 {
    pkt.extract(hdr.ipv4);
    transition select(hdr.ipv4.protocol) {
        17 : parse_udp;
        default: accept;
    }
  }

  state parse_udp {
    pkt.extract(hdr.udp);
    transition select(hdr.udp.dst_port) {
        4791: parse_ib_bth_2;
        default: accept;
    }
  }

  state parse_ib_bth_2 {
    pkt.extract(hdr.ib_bth);
    transition select(hdr.ib_bth.opcode) {
        12: parse_ib_reth_2;
        default: accept;
    }
  }

  state parse_ib_reth_2 {
      pkt.extract(hdr.ib_reth);
      pkt.extract(hdr.ib_icrc);

      transition select(hdr.ib_reth.rkey) {
          0: parse_fbt_2;
          default: accept;
      }
  }

  state parse_fbt_2 {
      pkt.extract(hdr.fbt);

      transition accept;
  }
}

/***************** M A T C H - A C T I O N *********************/

control Egress0(
        /* User */
        inout my_egress0_headers_t hdr, inout my_egress0_metadata_t meta,
        /* Intrinsic */
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_oport_md) {

    Hash<bit<32>>(HashAlgorithm_t.CRC32) crc32_hash;

    action calculate_crc32() {
        meta.crc32 = crc32_hash.get({
                meta.resv64a,

                hdr.ipv4.version,
                hdr.ipv4.ihl,
                meta.resv8b,
                hdr.ipv4.total_len,
                hdr.ipv4.identification,
                hdr.ipv4.flags,
                hdr.ipv4.frag_offset,
                meta.resv8c,
                hdr.ipv4.protocol,
                meta.resv16d,
                hdr.ipv4.src_addr,
                hdr.ipv4.dst_addr,

                hdr.udp.src_port,
                hdr.udp.dst_port,
                hdr.udp.len,
                meta.resv16e,

                hdr.ib_bth.opcode,
                hdr.ib_bth.event,
                hdr.ib_bth.migReq,
                hdr.ib_bth.padCount,
                hdr.ib_bth.version,
                hdr.ib_bth.pkey,
                meta.resv8f,
                hdr.ib_bth.dstQP,
                hdr.ib_bth.ackReq,
                hdr.ib_bth.resv7b,
                hdr.ib_bth.psn,

                hdr.ib_reth.va,
                hdr.ib_reth.rkey,
                hdr.ib_reth.length
        });
    }

    action fetch_stag_action(bit<32> stag) {
        meta.remote_stag = stag;
    }
    table fetch_stag_table {
        actions = {
            fetch_stag_action;
        }
    }

    action fetch_addr_action(bit<64> addr) {
        meta.remote_addr = addr;
    }
    table fetch_addr_table {
        actions = {
            fetch_addr_action;
        }
    }

    RECORD_ACTION(rkey_1, hdr.ib_reth.rkey, 32)
    RECORD_ACTION(va_1_1, hdr.ib_reth.va[63:32], 32)
    RECORD_ACTION(va_2_1, hdr.ib_reth.va[31:0], 32)

    RECORD_ACTION(rkey_2, hdr.ib_reth.rkey, 32)
    RECORD_ACTION(va_1_2, hdr.ib_reth.va[63:32], 32)
    RECORD_ACTION(va_2_2, hdr.ib_reth.va[31:0], 32)

    RECORD_ACTION(rkey_3, meta.remote_stag, 32)
    RECORD_ACTION(va_1_3, meta.remote_addr[63:32], 32)
    RECORD_ACTION(va_2_3, meta.remote_addr[31:0], 32)

    apply {
        if (hdr.fbt.isValid()) {

            fetch_stag_table.apply();
            fetch_addr_table.apply();

            hdr.ib_reth.rkey = meta.remote_stag;
            hdr.ib_reth.va = meta.remote_addr;

            meta.resv64a = 0xFFFFFFFFFFFFFFFF;
            meta.resv8b = 0xFF;
            meta.resv8c = 0xFF;
            meta.resv16d = 0xFFFF;
            meta.resv16e = 0xFFFF;
            meta.resv8f = 0xFF;

            calculate_crc32();

            meta.crc32revs[7:0] = meta.crc32[31:24];
            meta.crc32revs[15:8] = meta.crc32[23:16];
            meta.crc32revs[23:16] = meta.crc32[15:8];
            meta.crc32revs[31:24] = meta.crc32[7:0];

            hdr.ib_icrc.crc = meta.crc32revs;

            hdr.fbt.setInvalid();
        }
    }
}

/********************* D E P A R S E R ************************/

control
EgressDeparser0(packet_out pkt,
        /* User */
        inout my_egress0_headers_t hdr, in my_egress0_metadata_t meta,
        /* Intrinsic */
        in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
    apply { pkt.emit(hdr); }
}

/************ F I N A L P A C K A G E ******************************/
Pipeline(IngressParser1(), Ingress1(), IngressDeparser1(), EgressParser1(),
        Egress1(), EgressDeparser1()) pipe1;

Pipeline(IngressParser0(), Ingress0(), IngressDeparser0(), EgressParser0(),
        Egress0(), EgressDeparser0()) pipe0;

Switch(pipe0, pipe1) main;
