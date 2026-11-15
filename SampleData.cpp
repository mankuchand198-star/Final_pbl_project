#include "SampleData.h"
#include "Evidence/PhysicalEvidence.h"
#include "Evidence/DigitalEvidence.h"
#include "Evidence/TestimonialEvidence.h"

void loadSampleCase(CrimeCase& crimeCase, EvidenceList& evidenceList,
                     InvestigationQueue& eventQueue) {
    int seq = 1;

    crimeCase.setCaseInfo(
        "Burglary",
        "Sunrise Jewellers, MG Road",
        "12 Aug 2026, 11:30 PM",
        "Jewellery worth approximately Rs. 15,00,000 was stolen from a "
        "locked showroom. Entry appears to have been made through the "
        "back door without visible signs of forced damage to the main "
        "shutter."
    );
    eventQueue.enqueue(InvestigationEvent(seq++, "Case Created - 12 Aug 2026, 09:00 AM"));

    crimeCase.addSuspect(new Suspect(
        "Rahul Verma", 29, "Shop Assistant at Sunrise Jewellers",
        "Claims he was at home", 1, 2, 3, 3, 0,
        "I was at home with my family the whole night."
    ));
    crimeCase.addSuspect(new Suspect(
        "Aman Gupta", 34, "Delivery Driver",
        "On a night delivery run nearby", 2, 1, 2, 2, 1,
        "I was making a food delivery a few streets away."
    ));
    crimeCase.addSuspect(new Suspect(
        "Vijay Singh", 45, "Security Guard (off-duty that night)",
        "At a friend's house", 4, 1, 1, 1, 2,
        "I was at my friend Sanjay's house watching TV."
    ));
    crimeCase.addSuspect(new Suspect(
        "Rohit Malhotra", 24, "Nephew of a rival shop owner",
        "At a family wedding", 5, 0, 1, 0, 0,
        "I was attending my cousin's wedding with over 100 guests present."
    ));
    eventQueue.enqueue(InvestigationEvent(seq++, "4 Suspects Registered - 12 Aug 2026, 09:15 AM"));

    crimeCase.addWitness(new Witness(
        "Mrs. Kapoor", 52, "Neighbor",
        "I heard Rahul complaining about money troubles a few days before the theft.",
        "Lives next to the jewellery shop"
    ));
    crimeCase.addWitness(new Witness(
        "Sanjay Rathi", 44, "Friend of Vijay Singh",
        "Vijay was at my place that evening, though I can't say for sure how late.",
        "Vijay's alibi witness"
    ));

    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Collection Started - 13 Aug 2026, 10:00 AM"));

    Evidence* e1 = new DigitalEvidence(
        1, "CCTV footage from a neighboring shop shows a person matching "
           "Rahul's build near the back door at 11:15 PM.",
        Reliability::HIGH, true, "Neighboring shop CCTV", true
    );
    e1->addRelatedSuspect("Rahul Verma");
    evidenceList.addEvidence(e1);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E1 CCTV Footage - 13 Aug 2026, 10:05 AM"));

    Evidence* e2 = new PhysicalEvidence(
        2, "A set of duplicate shop keys was found in Rahul's backpack during a search.",
        Reliability::HIGH, true, "Rahul's backpack"
    );
    e2->addRelatedSuspect("Rahul Verma");
    evidenceList.addEvidence(e2);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E2 Duplicate Keys - 13 Aug 2026, 10:20 AM"));

    Evidence* e3 = new TestimonialEvidence(
        3, "A neighbor reports overhearing Rahul complain about money troubles "
           "and say he wanted to \"get back\" at the shop owner.",
        Reliability::MEDIUM, true, "Neighbor - Mrs. Kapoor"
    );
    e3->addRelatedSuspect("Rahul Verma");
    evidenceList.addEvidence(e3);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E3 Neighbor Testimony - 13 Aug 2026, 11:00 AM"));

    Evidence* e4 = new DigitalEvidence(
        4, "Delivery app GPS logs confirm Aman's route passed within 80m of "
           "the shop around the time of the crime.",
        Reliability::MEDIUM, true, "Delivery App GPS Logs", true
    );
    e4->addRelatedSuspect("Aman Gupta");
    evidenceList.addEvidence(e4);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E4 Delivery Logs - 13 Aug 2026, 11:30 AM"));

    Evidence* e5 = new TestimonialEvidence(
        5, "Aman's delivery manager confirms he completed all scheduled "
           "deliveries on time that night, leaving little spare time.",
        Reliability::MEDIUM, false, "Aman's Delivery Manager"
    );
    e5->addRelatedSuspect("Aman Gupta");
    evidenceList.addEvidence(e5);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E5 Manager Statement - 13 Aug 2026, 12:00 PM"));

    Evidence* e6 = new TestimonialEvidence(
        6, "Vijay's friend Sanjay partially confirms Vijay was at his house, "
           "but could not recall the exact hours.",
        Reliability::LOW, false, "Sanjay Rathi"
    );
    e6->addRelatedSuspect("Vijay Singh");
    evidenceList.addEvidence(e6);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E6 Friend Statement - 13 Aug 2026, 12:30 PM"));

    Evidence* e7 = new PhysicalEvidence(
        7, "Forensic examination of the scene found no fingerprints or "
           "footprints matching Vijay.",
        Reliability::MEDIUM, false, "Crime scene (no match found)"
    );
    e7->addRelatedSuspect("Vijay Singh");
    evidenceList.addEvidence(e7);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E7 Forensic Report - 13 Aug 2026, 01:00 PM"));

    Evidence* e8 = new TestimonialEvidence(
        8, "Multiple wedding guests and timestamped family photos confirm "
           "Rohit's presence at the wedding venue throughout the evening.",
        Reliability::HIGH, false, "Wedding Guests / Family"
    );
    e8->addRelatedSuspect("Rohit Malhotra");
    evidenceList.addEvidence(e8);
    eventQueue.enqueue(InvestigationEvent(seq++, "Evidence Added: E8 Wedding Confirmation - 13 Aug 2026, 01:30 PM"));

    eventQueue.enqueue(InvestigationEvent(seq++, "All Evidence Linked to Suspects - 13 Aug 2026, 02:00 PM"));
}
