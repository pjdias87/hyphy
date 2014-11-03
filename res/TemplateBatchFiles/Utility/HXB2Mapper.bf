skipCodeSelectionStep = 1;
ExecuteAFile			("../TemplateModels/chooseGeneticCode.def");
ApplyGeneticCodeTable  (0);

_HXB_env_offset					= 6224;

/* based on 
http://www.uniprot.org/uniprot/P04578
*/


_HXB_aa_offsets					= 
							    { "signal"	:0,
							      "c1"      :31,
								  "v1"		:130,
								  "v2"		:156,
								  "c2"		:196,
								  "v3"		:295,
								  "c3"		:330,
								  "v4"		:384,
								  "c4"		:418,
								  "v5"		:460,
								  "c5"      :471,
								  "fusion"  :511,
								  "gp41ecto":532,
								  "mper"    :661,
								  "gp41endo":683
								};

_HXB_env_region_name            = {{"signal","c1","v1","v2","c2","v3","c3","v4","c4","v5", "c5", "fusion", "gp41ecto", "mper", "gp41endo"}};
_HXB_aa_offset_matrix			= {{0,31,130,156,196,295,330,384,418,460,471,511,532,661,683,856}};
_HXB_env_upperbound             = _HXB_aa_offset_matrix [{{0,1}}][{{0,Columns(_HXB_env_region_name)}}];
								  
_HXB_Annotation = {};

for (_k = 0; _k < Columns (_HXB_env_region_name); _k += 1) {
    _rn = _HXB_env_region_name[_k];
    _HXB_Annotation [_rn] = {1,2};
    (_HXB_Annotation [_rn])[0] = _HXB_env_offset+ _HXB_aa_offset_matrix[_k]*3;
    (_HXB_Annotation [_rn])[1] = _HXB_env_offset+ _HXB_aa_offset_matrix[_k+1]*3-1;
}



_HXB2_Sequence_					= "TGGAAGGGCTAATTCACTCCCAACGAAGACAAGATATCCTTGATCTGTGGATCTACCACACACAAGGCTACTTCCCTGATTAGCAGAACTACACACCAGGGCCAGGGATCAGATATCCACTGACCTTTGGATGGTGCTACAAGCTAGTACCAGTTGAGCCAGAGAAGTTAGAAGAAGCCAACAAAGGAGAGAACACCAGCTTGTTACACCCTGTGAGCCTGCATGGAATGGATGACCCGGAGAGAGAAGTGTTAGAGTGGAGGTTTGACAGCCGCCTAGCATTTCATCACATGGCCCGAGAGCTGCATCCGGAGTACTTCAAGAACTGCTGACATCGAGCTTGCTACAAGGGACTTTCCGCTGGGGACTTTCCAGGGAGGCGTGGCCTGGGCGGGACTGGGGAGTGGCGAGCCCTCAGATCCTGCATATAAGCAGCTGCTTTTTGCCTGTACTGGGTCTCTCTGGTTAGACCAGATCTGAGCCTGGGAGCTCTCTGGCTAACTAGGGAACCCACTGCTTAAGCCTCAATAAAGCTTGCCTTGAGTGCTTCAAGTAGTGTGTGCCCGTCTGTTGTGTGACTCTGGTAACTAGAGATCCCTCAGACCCTTTTAGTCAGTGTGGAAAATCTCTAGCAGTGGCGCCCGAACAGGGACCTGAAAGCGAAAGGGAAACCAGAGGAGCTCTCTCGACGCAGGACTCGGCTTGCTGAAGCGCGCACGGCAAGAGGCGAGGGGCGGCGACTGGTGAGTACGCCAAAAATTTTGACTAGCGGAGGCTAGAAGGAGAGAGATGGGTGCGAGAGCGTCAGTATTAAGCGGGGGAGAATTAGATCGATGGGAAAAAATTCGGTTAAGGCCAGGGGGAAAGAAAAAATATAAATTAAAACATATAGTATGGGCAAGCAGGGAGCTAGAACGATTCGCAGTTAATCCTGGCCTGTTAGAAACATCAGAAGGCTGTAGACAAATACTGGGACAGCTACAACCATCCCTTCAGACAGGATCAGAAGAACTTAGATCATTATATAATACAGTAGCAACCCTCTATTGTGTGCATCAAAGGATAGAGATAAAAGACACCAAGGAAGCTTTAGACAAGATAGAGGAAGAGCAAAACAAAAGTAAGAAAAAAGCACAGCAAGCAGCAGCTGACACAGGACACAGCAATCAGGTCAGCCAAAATTACCCTATAGTGCAGAACATCCAGGGGCAAATGGTACATCAGGCCATATCACCTAGAACTTTAAATGCATGGGTAAAAGTAGTAGAAGAGAAGGCTTTCAGCCCAGAAGTGATACCCATGTTTTCAGCATTATCAGAAGGAGCCACCCCACAAGATTTAAACACCATGCTAAACACAGTGGGGGGACATCAAGCAGCCATGCAAATGTTAAAAGAGACCATCAATGAGGAAGCTGCAGAATGGGATAGAGTGCATCCAGTGCATGCAGGGCCTATTGCACCAGGCCAGATGAGAGAACCAAGGGGAAGTGACATAGCAGGAACTACTAGTACCCTTCAGGAACAAATAGGATGGATGACAAATAATCCACCTATCCCAGTAGGAGAAATTTATAAAAGATGGATAATCCTGGGATTAAATAAAATAGTAAGAATGTATAGCCCTACCAGCATTCTGGACATAAGACAAGGACCAAAGGAACCCTTTAGAGACTATGTAGACCGGTTCTATAAAACTCTAAGAGCCGAGCAAGCTTCACAGGAGGTAAAAAATTGGATGACAGAAACCTTGTTGGTCCAAAATGCGAACCCAGATTGTAAGACTATTTTAAAAGCATTGGGACCAGCGGCTACACTAGAAGAAATGATGACAGCATGTCAGGGAGTAGGAGGACCCGGCCATAAGGCAAGAGTTTTGGCTGAAGCAATGAGCCAAGTAACAAATTCAGCTACCATAATGATGCAGAGAGGCAATTTTAGGAACCAAAGAAAGATTGTTAAGTGTTTCAATTGTGGCAAAGAAGGGCACACAGCCAGAAATTGCAGGGCCCCTAGGAAAAAGGGCTGTTGGAAATGTGGAAAGGAAGGACACCAAATGAAAGATTGTACTGAGAGACAGGCTAATTTTTTAGGGAAGATCTGGCCTTCCTACAAGGGAAGGCCAGGGAATTTTCTTCAGAGCAGACCAGAGCCAACAGCCCCACCAGAAGAGAGCTTCAGGTCTGGGGTAGAGACAACAACTCCCCCTCAGAAGCAGGAGCCGATAGACAAGGAACTGTATCCTTTAACTTCCCTCAGGTCACTCTTTGGCAACGACCCCTCGTCACAATAAAGATAGGGGGGCAACTAAAGGAAGCTCTATTAGATACAGGAGCAGATGATACAGTATTAGAAGAAATGAGTTTGCCAGGAAGATGGAAACCAAAAATGATAGGGGGAATTGGAGGTTTTATCAAAGTAAGACAGTATGATCAGATACTCATAGAAATCTGTGGACATAAAGCTATAGGTACAGTATTAGTAGGACCTACACCTGTCAACATAATTGGAAGAAATCTGTTGACTCAGATTGGTTGCACTTTAAATTTTCCCATTAGCCCTATTGAGACTGTACCAGTAAAATTAAAGCCAGGAATGGATGGCCCAAAAGTTAAACAATGGCCATTGACAGAAGAAAAAATAAAAGCATTAGTAGAAATTTGTACAGAGATGGAAAAGGAAGGGAAAATTTCAAAAATTGGGCCTGAAAATCCATACAATACTCCAGTATTTGCCATAAAGAAAAAAGACAGTACTAAATGGAGAAAATTAGTAGATTTCAGAGAACTTAATAAGAGAACTCAAGACTTCTGGGAAGTTCAATTAGGAATACCACATCCCGCAGGGTTAAAAAAGAAAAAATCAGTAACAGTACTGGATGTGGGTGATGCATATTTTTCAGTTCCCTTAGATGAAGACTTCAGGAAGTATACTGCATTTACCATACCTAGTATAAACAATGAGACACCAGGGATTAGATATCAGTACAATGTGCTTCCACAGGGATGGAAAGGATCACCAGCAATATTCCAAAGTAGCATGACAAAAATCTTAGAGCCTTTTAGAAAACAAAATCCAGACATAGTTATCTATCAATACATGGATGATTTGTATGTAGGATCTGACTTAGAAATAGGGCAGCATAGAACAAAAATAGAGGAGCTGAGACAACATCTGTTGAGGTGGGGACTTACCACACCAGACAAAAAACATCAGAAAGAACCTCCATTCCTTTGGATGGGTTATGAACTCCATCCTGATAAATGGACAGTACAGCCTATAGTGCTGCCAGAAAAAGACAGCTGGACTGTCAATGACATACAGAAGTTAGTGGGGAAATTGAATTGGGCAAGTCAGATTTACCCAGGGATTAAAGTAAGGCAATTATGTAAACTCCTTAGAGGAACCAAAGCACTAACAGAAGTAATACCACTAACAGAAGAAGCAGAGCTAGAACTGGCAGAAAACAGAGAGATTCTAAAAGAACCAGTACATGGAGTGTATTATGACCCATCAAAAGACTTAATAGCAGAAATACAGAAGCAGGGGCAAGGCCAATGGACATATCAAATTTATCAAGAGCCATTTAAAAATCTGAAAACAGGAAAATATGCAAGAATGAGGGGTGCCCACACTAATGATGTAAAACAATTAACAGAGGCAGTGCAAAAAATAACCACAGAAAGCATAGTAATATGGGGAAAGACTCCTAAATTTAAACTGCCCATACAAAAGGAAACATGGGAAACATGGTGGACAGAGTATTGGCAAGCCACCTGGATTCCTGAGTGGGAGTTTGTTAATACCCCTCCCTTAGTGAAATTATGGTACCAGTTAGAGAAAGAACCCATAGTAGGAGCAGAAACCTTCTATGTAGATGGGGCAGCTAACAGGGAGACTAAATTAGGAAAAGCAGGATATGTTACTAATAGAGGAAGACAAAAAGTTGTCACCCTAACTGACACAACAAATCAGAAGACTGAGTTACAAGCAATTTATCTAGCTTTGCAGGATTCGGGATTAGAAGTAAACATAGTAACAGACTCACAATATGCATTAGGAATCATTCAAGCACAACCAGATCAAAGTGAATCAGAGTTAGTCAATCAAATAATAGAGCAGTTAATAAAAAAGGAAAAGGTCTATCTGGCATGGGTACCAGCACACAAAGGAATTGGAGGAAATGAACAAGTAGATAAATTAGTCAGTGCTGGAATCAGGAAAGTACTATTTTTAGATGGAATAGATAAGGCCCAAGATGAACATGAGAAATATCACAGTAATTGGAGAGCAATGGCTAGTGATTTTAACCTGCCACCTGTAGTAGCAAAAGAAATAGTAGCCAGCTGTGATAAATGTCAGCTAAAAGGAGAAGCCATGCATGGACAAGTAGACTGTAGTCCAGGAATATGGCAACTAGATTGTACACATTTAGAAGGAAAAGTTATCCTGGTAGCAGTTCATGTAGCCAGTGGATATATAGAAGCAGAAGTTATTCCAGCAGAAACAGGGCAGGAAACAGCATATTTTCTTTTAAAATTAGCAGGAAGATGGCCAGTAAAAACAATACATACTGACAATGGCAGCAATTTCACCGGTGCTACGGTTAGGGCCGCCTGTTGGTGGGCGGGAATCAAGCAGGAATTTGGAATTCCCTACAATCCCCAAAGTCAAGGAGTAGTAGAATCTATGAATAAAGAATTAAAGAAAATTATAGGACAGGTAAGAGATCAGGCTGAACATCTTAAGACAGCAGTACAAATGGCAGTATTCATCCACAATTTTAAAAGAAAAGGGGGGATTGGGGGGTACAGTGCAGGGGAAAGAATAGTAGACATAATAGCAACAGACATACAAACTAAAGAATTACAAAAACAAATTACAAAAATTCAAAATTTTCGGGTTTATTACAGGGACAGCAGAAATCCACTTTGGAAAGGACCAGCAAAGCTCCTCTGGAAAGGTGAAGGGGCAGTAGTAATACAAGATAATAGTGACATAAAAGTAGTGCCAAGAAGAAAAGCAAAGATCATTAGGGATTATGGAAAACAGATGGCAGGTGATGATTGTGTGGCAAGTAGACAGGATGAGGATTAGAACATGGAAAAGTTTAGTAAAACACCATATGTATGTTTCAGGGAAAGCTAGGGGATGGTTTTATAGACATCACTATGAAAGCCCTCATCCAAGAATAAGTTCAGAAGTACACATCCCACTAGGGGATGCTAGATTGGTAATAACAACATATTGGGGTCTGCATACAGGAGAAAGAGACTGGCATTTGGGTCAGGGAGTCTCCATAGAATGGAGGAAAAAGAGATATAGCACACAAGTAGACCCTGAACTAGCAGACCAACTAATTCATCTGTATTACTTTGACTGTTTTTCAGACTCTGCTATAAGAAAGGCCTTATTAGGACACATAGTTAGCCCTAGGTGTGAATATCAAGCAGGACATAACAAGGTAGGATCTCTACAATACTTGGCACTAGCAGCATTAATAACACCAAAAAAGATAAAGCCACCTTTGCCTAGTGTTACGAAACTGACAGAGGATAGATGGAACAAGCCCCAGAAGACCAAGGGCCACAGAGGGAGCCACACAATGAATGGACACTAGAGCTTTTAGAGGAGCTTAAGAATGAAGCTGTTAGACATTTTCCTAGGATTTGGCTCCATGGCTTAGGGCAACATATCTATGAAACTTATGGGGATACTTGGGCAGGAGTGGAAGCCATAATAAGAATTCTGCAACAACTGCTGTTTATCCATTTTCAGAATTGGGTGTCGACATAGCAGAATAGGCGTTACTCGACAGAGGAGAGCAAGAAATGGAGCCAGTAGATCCTAGACTAGAGCCCTGGAAGCATCCAGGAAGTCAGCCTAAAACTGCTTGTACCAATTGCTATTGTAAAAAGTGTTGCTTTCATTGCCAAGTTTGTTTCATAACAAAAGCCTTAGGCATCTCCTATGGCAGGAAGAAGCGGAGACAGCGACGAAGAGCTCATCAGAACAGTCAGACTCATCAAGCTTCTCTATCAAAGCAGTAAGTAGTACATGTAACGCAACCTATACCAATAGTAGCAATAGTAGCATTAGTAGTAGCAATAATAATAGCAATAGTTGTGTGGTCCATAGTAATCATAGAATATAGGAAAATATTAAGACAAAGAAAAATAGACAGGTTAATTGATAGACTAATAGAAAGAGCAGAAGACAGTGGCAATGAGAGTGAAGGAGAAATATCAGCACTTGTGGAGATGGGGGTGGAGATGGGGCACCATGCTCCTTGGGATGTTGATGATCTGTAGTGCTACAGAAAAATTGTGGGTCACAGTCTATTATGGGGTACCTGTGTGGAAGGAAGCAACCACCACTCTATTTTGTGCATCAGATGCTAAAGCATATGATACAGAGGTACATAATGTTTGGGCCACACATGCCTGTGTACCCACAGACCCCAACCCACAAGAAGTAGTATTGGTAAATGTGACAGAAAATTTTAACATGTGGAAAAATGACATGGTAGAACAGATGCATGAGGATATAATCAGTTTATGGGATCAAAGCCTAAAGCCATGTGTAAAATTAACCCCACTCTGTGTTAGTTTAAAGTGCACTGATTTGAAGAATGATACTAATACCAATAGTAGTAGCGGGAGAATGATAATGGAGAAAGGAGAGATAAAAAACTGCTCTTTCAATATCAGCACAAGCATAAGAGGTAAGGTGCAGAAAGAATATGCATTTTTTTATAAACTTGATATAATACCAATAGATAATGATACTACCAGCTATAAGTTGACAAGTTGTAACACCTCAGTCATTACACAGGCCTGTCCAAAGGTATCCTTTGAGCCAATTCCCATACATTATTGTGCCCCGGCTGGTTTTGCGATTCTAAAATGTAATAATAAGACGTTCAATGGAACAGGACCATGTACAAATGTCAGCACAGTACAATGTACACATGGAATTAGGCCAGTAGTATCAACTCAACTGCTGTTAAATGGCAGTCTAGCAGAAGAAGAGGTAGTAATTAGATCTGTCAATTTCACGGACAATGCTAAAACCATAATAGTACAGCTGAACACATCTGTAGAAATTAATTGTACAAGACCCAACAACAATACAAGAAAAAGAATCCGTATCCAGAGAGGACCAGGGAGAGCATTTGTTACAATAGGAAAAATAGGAAATATGAGACAAGCACATTGTAACATTAGTAGAGCAAAATGGAATAACACTTTAAAACAGATAGCTAGCAAATTAAGAGAACAATTTGGAAATAATAAAACAATAATCTTTAAGCAATCCTCAGGAGGGGACCCAGAAATTGTAACGCACAGTTTTAATTGTGGAGGGGAATTTTTCTACTGTAATTCAACACAACTGTTTAATAGTACTTGGTTTAATAGTACTTGGAGTACTGAAGGGTCAAATAACACTGAAGGAAGTGACACAATCACCCTCCCATGCAGAATAAAACAAATTATAAACATGTGGCAGAAAGTAGGAAAAGCAATGTATGCCCCTCCCATCAGTGGACAAATTAGATGTTCATCAAATATTACAGGGCTGCTATTAACAAGAGATGGTGGTAATAGCAACAATGAGTCCGAGATCTTCAGACCTGGAGGAGGAGATATGAGGGACAATTGGAGAAGTGAATTATATAAATATAAAGTAGTAAAAATTGAACCATTAGGAGTAGCACCCACCAAGGCAAAGAGAAGAGTGGTGCAGAGAGAAAAAAGAGCAGTGGGAATAGGAGCTTTGTTCCTTGGGTTCTTGGGAGCAGCAGGAAGCACTATGGGCGCAGCCTCAATGACGCTGACGGTACAGGCCAGACAATTATTGTCTGGTATAGTGCAGCAGCAGAACAATTTGCTGAGGGCTATTGAGGCGCAACAGCATCTGTTGCAACTCACAGTCTGGGGCATCAAGCAGCTCCAGGCAAGAATCCTGGCTGTGGAAAGATACCTAAAGGATCAACAGCTCCTGGGGATTTGGGGTTGCTCTGGAAAACTCATTTGCACCACTGCTGTGCCTTGGAATGCTAGTTGGAGTAATAAATCTCTGGAACAGATTTGGAATCACACGACCTGGATGGAGTGGGACAGAGAAATTAACAATTACACAAGCTTAATACACTCCTTAATTGAAGAATCGCAAAACCAGCAAGAAAAGAATGAACAAGAATTATTGGAATTAGATAAATGGGCAAGTTTGTGGAATTGGTTTAACATAACAAATTGGCTGTGGTATATAAAATTATTCATAATGATAGTAGGAGGCTTGGTAGGTTTAAGAATAGTTTTTGCTGTACTTTCTATAGTGAATAGAGTTAGGCAGGGATATTCACCATTATCGTTTCAGACCCACCTCCCAACCCCGAGGGGACCCGACAGGCCCGAAGGAATAGAAGAAGAAGGTGGAGAGAGAGACAGAGACAGATCCATTCGATTAGTGAACGGATCCTTGGCACTTATCTGGGACGATCTGCGGAGCCTGTGCCTCTTCAGCTACCACCGCTTGAGAGACTTACTCTTGATTGTAACGAGGATTGTGGAACTTCTGGGACGCAGGGGGTGGGAAGCCCTCAAATATTGGTGGAATCTCCTACAGTATTGGAGTCAGGAACTAAAGAATAGTGCTGTTAGCTTGCTCAATGCCACAGCCATAGCAGTAGCTGAGGGGACAGATAGGGTTATAGAAGTAGTACAAGGAGCTTGTAGAGCTATTCGCCACATACCTAGAAGAATAAGACAGGGCTTGGAAAGGATTTTGCTATAAGATGGGTGGCAAGTGGTCAAAAAGTAGTGTGATTGGATGGCCTACTGTAAGGGAAAGAATGAGACGAGCTGAGCCAGCAGCAGATAGGGTGGGAGCAGCATCTCGAGACCTGGAAAAACATGGAGCAATCACAAGTAGCAATACAGCAGCTACCAATGCTGCTTGTGCCTGGCTAGAAGCACAAGAGGAGGAGGAGGTGGGTTTTCCAGTCACACCTCAGGTACCTTTAAGACCAATGACTTACAAGGCAGCTGTAGATCTTAGCCACTTTTTAAAAGAAAAGGGGGGACTGGAAGGGCTAATTCACTCCCAAAGAAGACAAGATATCCTTGATCTGTGGATCTACCACACACAAGGCTACTTCCCTGATTAGCAGAACTACACACCAGGGCCAGGGGTCAGATATCCACTGACCTTTGGATGGTGCTACAAGCTAGTACCAGTTGAGCCAGATAAGATAGAAGAGGCCAATAAAGGAGAGAACACCAGCTTGTTACACCCTGTGAGCCTGCATGGGATGGATGACCCGGAGAGAGAAGTGTTAGAGTGGAGGTTTGACAGCCGCCTAGCATTTCATCACGTGGCCCGAGAGCTGCATCCGGAGTACTTCAAGAACTGCTGACATCGAGCTTGCTACAAGGGACTTTCCGCTGGGGACTTTCCAGGGAGGCGTGGCCTGGGCGGGACTGGGGAGTGGCGAGCCCTCAGATCCTGCATATAAGCAGCTGCTTTTTGCCTGTACTGGGTCTCTCTGGTTAGACCAGATCTGAGCCTGGGAGCTCTCTGGCTAACTAGGGAACCCACTGCTTAAGCCTCAATAAAGCTTGCCTTGAGTGCTTCAAGTAGTGTGTGCCCGTCTGTTGTGTGACTCTGGTAACTAGAGATCCCTCAGACCCTTTTAGTCAGTGTGGAAAATCTCTAGCA";
_HXB2_Env_Sequence_				= _HXB2_Sequence_ [_HXB_env_offset][_HXB_env_offset+2567];

_HXB2_AA_						= translateCodonToAA (_HXB2_Sequence_,defineCodonToAA(),2);
_HXB2_AA_ENV_					= translateCodonToAA (_HXB2_Env_Sequence_,defineCodonToAA(),0);

_hxb_alignOptions_nuc = {};
_hxb_alignOptions_nuc ["SEQ_ALIGN_CHARACTER_MAP"]="ACGT";
_hxb_alignOptions_nuc ["SEQ_ALIGN_SCORE_MATRIX"] = 	{
{5,-4,-4,-4}
{-4,5,-4,-4}
{-4,-4,5,-4}
{-4,-4,-4,5}
};
_hxb_alignOptions_nuc ["SEQ_ALIGN_GAP_OPEN"]	= 	50;
_hxb_alignOptions_nuc ["SEQ_ALIGN_GAP_OPEN2"]	= 	50;
_hxb_alignOptions_nuc ["SEQ_ALIGN_GAP_EXTEND"]	= 	1;
_hxb_alignOptions_nuc ["SEQ_ALIGN_GAP_EXTEND2"]	= 	1;
_hxb_alignOptions_nuc ["SEQ_ALIGN_AFFINE"]		=   1;
_hxb_alignOptions_nuc ["SEQ_ALIGN_NO_TP"]		=   1;


LoadFunctionLibrary ("SeqAlignShared", {"00": "HIV 5%", "01": "No penalty", "02": "First in file" });

_hxb_alignOptions_prot = alignOptions;

scoreMatrix = _hxb_alignOptions_prot ["SEQ_ALIGN_SCORE_MATRIX"];
LoadFunctionLibrary ("SeqAlignmentCodonShared", {"00": "HIV 25%", "01": "No penalty", "02": "First in file" });
_hxb_alignOptions_codon = alignOptions;


/*-------------------------------------------------------------*/
function mapSequenceToHXB2 (seq,option)
/* 
	option 0 - nucleotide alignment
    option 1 - amino-acid alignment
    option 2 - codon alignment
*/
{
 	if (option == 1)
 	{
		return mapSequenceToHXB2Aux (seq, _HXB2_AA_, option);
 	}
	return mapSequenceToHXB2Aux (seq, _HXB2_Sequence_, option);
}

/*-------------------------------------------------------------*/
function mapSequenceToHXB2Aux (seq,ref,option)
/* 
	option 0 - nucleotide alignment
    option 1 - amino-acid alignment
    option 2 - codon alignment
*/
{
	_seqLen	  = Abs(seq);
	_coordMap = {_seqLen,1};
	
	if (option != 1)
	{
		_inStr 		 = {{ref,seq}};
        if (option == 0)
        {
            AlignSequences(aligned, _inStr, _hxb_alignOptions_nuc);
        }
        else
        {
             AlignSequences(aligned, _inStr, _hxb_alignOptions_codon);
        }
    }
	else
	{
		_inStr 		 = {{ref,seq}};
		AlignSequences(aligned, _inStr, _hxb_alignOptions_prot);
	
	}
	
	_alignedHXB  = (aligned[0])[1];
	_alignedQRY  = (aligned[0])[2];
	
	_k				= (_alignedHXB$"^\\-+");
	_referenceSpan	= _k[1]+1;
	
	for (_k = 0; _k < _referenceSpan; _k = _k+1)
	{
		_coordMap[_k] = 0;
	}
	
	_qryCoord = _k;
	_refCoord = 0;

	while (_k < Abs(_alignedQRY))
	{
		if (_alignedQRY[_k] != "-")
		{
			_coordMap[_qryCoord] = _refCoord;
			_qryCoord = _qryCoord + 1;
		}
		if (_alignedHXB[_k] != "-")
		{
			_refCoord = _refCoord + 1;
		}
		_k = _k+1;
	}
	return _coordMap;
}

/*-------------------------------------------------------------*/
function selectHXB2subsequenceAux (seq,theSubset,mode)
{
    _template = {1,Abs(_HXB2_Sequence_)};
	_k2		 = Rows(theSubset)*Columns(theSubset);
	for (_k = 0; _k < _k2; _k = _k+1)
	{
		_span = _HXB_Annotation[theSubset[_k]];
		if (Abs(_span))
		{
			_template += _template["_MATRIX_ELEMENT_COLUMN_>=_span[0]&&_MATRIX_ELEMENT_COLUMN_<=_span[1]"];
		}
	}
	_mappedReference = mapSequenceToHXB2 (seq,0+2*mode);
	_subset			 = ""; _subset * 256;
	_k2 = Rows(_mappedReference);
	_k4 = Columns (_template);
	
    for (_k = 0; _k < _k2; _k = _k+1)
	{
		_k3 = _mappedReference[_k];
		if (_k3 >= _k4)
		{
			break;
		}
		if (_template[_k3])
		{
			_subset * seq[_k];
		}
	}
    
	_subset * 0;
	return _subset;

}

/*-------------------------------------------------------------*/
function selectHXB2subsequence (seq,theSubset)
{
    return selectHXB2subsequenceAux(seq,theSubset,0);
}

/*-------------------------------------------------------------*/
function selectHXB2subsequenceCodon (seq,theSubset)
{
    return selectHXB2subsequenceAux(seq,theSubset,1);
}

//--------------------------------------------------------------------------------

function		isoElectricPoint (seq) {
	COUNT_GAPS_IN_FREQUENCIES = 0;
	
	DataSet 			protSeq = ReadFromString ("$BASESET:BASE20\n>1\n" + seq);
	DataSetFilter		protFil = CreateFilter	 (protSeq,1);
	
	HarvestFrequencies (freqs,protFil,1,1,1);
	
	freqs = freqs*protFil.sites;
	
	expression = ""  + freqs[6 ] + "/(1+10^(pH-6.04))"  + /* H */
				 "+" + freqs[8 ] + "/(1+10^(pH-10.54))" + /* K */
				 "+" + freqs[14] + "/(1+10^(pH-12.48))" + /* R */
				 
				 "-" + freqs[2 ] + "/(1+10^(3.9-pH))"   + /* D */
				 "-" + freqs[3 ] + "/(1+10^(4.07-pH))"   + /* E */
				 "-" + freqs[1 ] + "/(1+10^(8.18-pH))"   + /* C */
				 "-" + freqs[19] + "/(1+10^(10.46-pH))"   ; /* Y */
	
	pH :> 0;
	pH :< 14;
	pH = 6.5;
	

	ExecuteCommands ("function ComputePI (pH){ return -Abs(`expression`); }");
	Optimize 		(res, ComputePI(pH));
		
	return res[0][0];
}


//--------------------------------------------------------------------------------

lfunction		countPNGS		(seq){
    pngs = seq || "N\\-*[^P]\\-*[ST]\\-*[^P]";
	return Rows(pngs)/2 - (pngs[0] < 0) ;
}

/*-------------------------------------------------------------*/
function partitionENVsequence (seq, nucOrAA) {


    if (nucOrAA != 1) {
		_mappedReference = mapSequenceToHXB2Aux (seq,_HXB2_Env_Sequence_,nucOrAA);
		_binned_regions = _HXB_env_upperbound * 3;
	}
	else {
		_mappedReference = mapSequenceToHXB2Aux (seq,_HXB2_AA_ENV_,nucOrAA);
		_binned_regions = _HXB_env_upperbound;
	}
	
	_allPartitions       = Columns (_HXB_env_region_name);
	_currentIndex        = 0;
	_upperBound          = _binned_regions[_currentIndex];
	_mappedLength        = Rows (_mappedReference);
	_splitSequence       = {};
	
    for (_currentIndex = 0; _currentIndex < _mappedLength; _currentIndex+=1) {
        if (_mappedReference[_currentIndex] > 0) {
           if (_currentIndex > 0) {
                _currentIndex += (-1);
           }
           break;
        }
    }
	
	for (_currentPartition    = 0; _currentPartition < _allPartitions; _currentPartition += 1) {
	    _segmentName = _HXB_env_region_name[_currentPartition];
	    _splitSequence [_segmentName] = "";
	    _splitSequence [_segmentName] * 128;
	}
	_currentPartition = 0;
	
	while (_currentPartition < _allPartitions && _currentIndex < _mappedLength) {
	    if (_mappedReference[_currentIndex] >= _upperBound && _currentPartition < _allPartitions - 1) {
	        _currentPartition += 1;
	        _upperBound          = _binned_regions[_currentPartition];
	    } 
	    _splitSequence [_HXB_env_region_name[_currentPartition]] * seq[_currentIndex];
	    _currentIndex += 1;
	}

	for (_currentPartition    = 0; _currentPartition < _allPartitions; _currentPartition += 1) {
        _splitSequence [_HXB_env_region_name[_currentPartition]] * 0;
	}	
		
		
		
	return _splitSequence;
}

/*-------------------------------------------------------------*/
function selectHXB2ENVsubsequence (seq,theSubset, nucOrAA) {
	if (nucOrAA != 1)
	{
		_template = {1,Abs(_HXB2_Env_Sequence_)};
	}
	else
	{
		_template = {1,Abs(_HXB2_AA_ENV_)};	
	}
	
	_k2		  = Rows(theSubset)*Columns(theSubset);
	for (_k = 0; _k < _k2; _k = _k+1)
	{
		_span = _HXB_Annotation[theSubset[_k]] + (- _HXB_env_offset);
		
		if (Abs(_span))
		{
			if (nucOrAA == 0)
			{
				_template += _template["_MATRIX_ELEMENT_COLUMN_>=_span[0]&&_MATRIX_ELEMENT_COLUMN_<=_span[1]"];
			}
			else
			{
				_template += _template["_MATRIX_ELEMENT_COLUMN_>=_span[0]$3&&_MATRIX_ELEMENT_COLUMN_<=_span[1]$3"];			
			}
		}
	}
		
		
    if (nucOrAA != 1)
    {
        _mappedReference = mapSequenceToHXB2Aux (seq,_HXB2_Env_Sequence_,nucOrAA);
    }
    else
    {
        _mappedReference = mapSequenceToHXB2Aux (seq,_HXB2_AA_ENV_,nucOrAA);
    }
	
	_subset			 = ""; _subset * 256;
	
	_k2 = Rows(_mappedReference);
	_k4 = Columns (_template);
	for (_k = 0; _k < _k2; _k = _k+1)
	{
		_k3 = _mappedReference[_k];
		if (_k3 >= _k4)
		{
			break;
		}
		if (_template[_k3])
		{
			_subset * seq[_k];
		}
	}

	_subset * 0;
	return _subset;
}


/*-------------------------------------------------------------*/
function fractionalHXB2map (seq)
{
	_inStr 		 = {{_HXB2_AA_ENV_,seq}};
	
	AlignSequences(aligned, _inStr, _hxb_alignOptions_prot);
	
		
	_alignedHXB  = (aligned[0])[1];
	_alignedQRY  = (aligned[0])[2];
	
	_seqLen	  = Abs(seq);
	_coordMap = {_seqLen,1};
	
	
	_currentRegionHXBSpan	= 0;
	_currentRegionIndex		= 1;
	
	_currentHXB2Index		= 0;
	_currentQRYIndex		= 0;
	_k2						= Abs (_alignedHXB);
	
	for (_k = 0; _k < _k2; _k+=1)
	{
	
		//fprintf (stdout, _k, ":", _currentHXB2Index,_alignedHXB[_k], "|",  _alignedQRY[_k], _currentQRYIndex,  "\n");
			
		if (_alignedHXB[_k] != "-")
		{
			_currentHXB2Index += 1;
		}
		if (_alignedQRY[_k] != "-")
		{
			_currentQRYIndex += 1;
		}
		
		if (_currentHXB2Index == _HXB_aa_offset_matrix [_currentRegionIndex])
		{
		
			_qrySpan			 = _currentQRYIndex - _lastQRYRegion;
			_hxbSpan			 = _HXB_aa_offset_matrix [_currentRegionIndex] - _HXB_aa_offset_matrix [_currentRegionIndex-1];
			_scaler				 = (_hxbSpan-1)/(_qrySpan-1);
			_hxbSpan			 = _HXB_aa_offset_matrix [_currentRegionIndex-1];
			//fprintf (stdout, "In offset ",_currentRegionIndex,"\n", _scaler, ":", _hxbSpan, ":", _currentQRYIndex, ":", _lastQRYRegion, "\n");
			
			_currentRegionIndex += 1;	
			if (_currentRegionIndex == Columns(_HXB_aa_offset_matrix))
			{
				_k				 = _k2;
				_currentQRYIndex = Abs(seq);
			}
			
			for (_k3 = _lastQRYRegion; _k3 < _currentQRYIndex; _k3 += 1)
			{
				_coordMap [_k3] = _hxbSpan + (_k3-_lastQRYRegion) * _scaler;
				//fprintf (stdout, "\t", _k3, ":", _coordMap [_k3], "\n");
			}
			
			
			_lastQRYRegion       = _currentQRYIndex;
		}

	}
	
	return _coordMap;
}



