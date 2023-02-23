// $Id$
// $HeadURL$
//
//    File: DDAQAddress.h
// Created: Mon Jun 24 10:11:30 EDT 2013
// Creator: davidl (on Darwin harriet.jlab.org 11.4.2 i386)
//

#ifndef _DDAQAddress_
#define _DDAQAddress_

#include <JANA/JObject.h>

//--------------------------------------------------------------
/// This class holds the Crate, Slot, Channel address for
/// a digitized value from the DAQ system. It is a base class
/// for the low-level hit objects generated by the DAQ system.
///
/// It also holds the trigger number for cases when this hit
/// was read in a multi-event block (i.e. "entangled" or "blocked"
/// events). The value of itrigger will always be zero for DAQ
/// containing a single L1-triggered event.
///
//--------------------------------------------------------------
class DDAQAddress:public JObject{
	public:
		JOBJECT_PUBLIC(DDAQAddress);
		DDAQAddress(uint32_t rocid=0, uint32_t slot=0, uint32_t channel=0, uint32_t itrigger=0):rocid(rocid),slot(slot),channel(channel),itrigger(itrigger){};
		virtual ~DDAQAddress(){}
		
		uint32_t rocid;    // crate
		uint32_t slot;     // slot
		uint32_t channel;  // channel
		uint32_t itrigger; // trigger number within block (starting from 0)
		
		bool operator==(const DDAQAddress &d){
			if(d.rocid    != rocid   ) return false;
			if(d.slot     != slot    ) return false;
			if(d.channel  != channel ) return false;
			if(d.itrigger != itrigger) return false;
			return true;
		}
				
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void Summarize(JObjectSummary& summary) const override {
			summary.add(rocid, NAME_OF(rocid), "%d");
			summary.add(slot, NAME_OF(slot), "%d");
			summary.add(channel, NAME_OF(channel), "%d");
			summary.add(itrigger, NAME_OF(itrigger), "%d");
		}
};

#endif // _DDAQAddress_
