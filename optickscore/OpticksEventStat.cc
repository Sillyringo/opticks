/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */


#include <iostream>
#include "NPY.hpp"
#include "RecordsNPY.hpp"
#include "Opticks.hh"
#include "OpticksEvent.hh"
#include "OpticksPhoton.hh"
#include "OpticksFlags.hh"
#include "OpticksEventStat.hh"

#include "SLOG.hh"



OpticksEventStat::OpticksEventStat(OpticksEvent* evt, unsigned num_cat)
    :
    m_ok(evt->getOpticks()),
    m_evt(evt),
    m_num_cat(num_cat),
    m_noload(evt->isNoLoad()),
    m_records(evt->getRecordsNPY()),
    m_pho(evt->getPhotonData()),
    m_seq(evt->getSequenceData()),
    m_pho_num(m_pho ? m_pho->getShape(0) : 0),
    m_seq_num(m_seq ? m_seq->getShape(0) : 0),
    m_counts(new MQC[num_cat]), 
    m_totmin(2)
{
    init();

 
}


void OpticksEventStat::init()
{
    assert(m_ok);
    countTotals();
}


void OpticksEventStat::increment(unsigned cat, unsigned long long seqhis_)
{
    assert( cat < m_num_cat );
    m_counts[cat][seqhis_]++ ; 
}


void OpticksEventStat::countTotals()
{
    for(unsigned i=0 ; i < m_pho_num ; i++)
    {  
        unsigned long long seqhis_ = m_seq->getValue(i,0,0);
        m_total[seqhis_]++;   // <- same for all trees
    }
}


void OpticksEventStat::dump(const char* msg)
{
    LOG(info) << msg
              << " evt " << m_evt->brief()
              << " totmin " << m_totmin  
              ;

    // copy map into vector of pairs to allow sort into descending tot order
    VPQC vpqc ; 
    for(MQC::const_iterator it=m_total.begin() ; it != m_total.end() ; it++) vpqc.push_back(PQC(it->first,it->second));
    std::sort( vpqc.begin(), vpqc.end(), PQC_desc ); 


    for(VPQC::const_iterator it=vpqc.begin() ; it != vpqc.end() ; it++)
    {
        unsigned long long _seqhis = it->first ; 
        unsigned tot_ = it->second ; 
        if(tot_ < m_totmin ) continue ; 

        std::cout 
             << " seqhis " << std::setw(16) << std::hex << _seqhis << std::dec
             << " " << std::setw(64) << OpticksPhoton::FlagSequence( _seqhis, true )
             << " tot " << std::setw(6) << tot_ 
             ;

        if(m_num_cat > 0)
        {
            std::cout << " cat ( "  ;
            for(unsigned cat=0 ; cat < m_num_cat ; cat++ ) std::cout << " " << std::setw(6) << m_counts[cat][_seqhis]  ;
            std::cout << " ) " ;

            std::cout << " frac ( "  ;
            for(unsigned cat=0 ; cat < m_num_cat ; cat++ ) std::cout << " " << std::setw(6) << float(m_counts[cat][_seqhis])/float(tot_)  ;
            std::cout << " ) " ;
        }

        std::cout << std::endl ; 
    }
}




