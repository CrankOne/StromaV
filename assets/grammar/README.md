# The Detector Selector micro-language

This micro-language offers a convinient way to compose the objects providing
run-time filtering of events based on the detector descriptor.

    =ECAL0          (* Selects ECAL0 only by major number *)
    ECAL{0}         (* Same *)
    ECAL{*}         (* Selects all ECAL detector instances in family *)
    ECAL{0,1}       (* Selects ECAL: preshower, main part and ECALSUM *)
    ECAL{"SUM"}     (* Selects ECALSUM only *)
    HCAL{0-3}       (* Selects all HCAL modules (0, 1, 2, 3) *)
    HCAL{0-3!1}     (* Selects all HCAL modules except for 1: 0, 2, 3 *)
    HCAL{<2}        (* Selects HCAL modules 0 and 2 *)

    =MYDET1
    MYDET{<2x2}
    MYDET{5x4-6x1}


