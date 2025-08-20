/* ------------------------------------------------------------
author: "GRAME"
name: "reverb_faust"
version: "0.1"
Code generated with Faust 2.81.2 (https://faust.grame.fr)
Compilation options: -lang cpp -rui -nvi -ct 1 -cn _reverb_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0
------------------------------------------------------------ */

#ifndef  ___reverb_faust_H__
#define  ___reverb_faust_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS _reverb_faust
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

const static int i_reverb_faustSIG0Wave0[2048] = {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997,1009,1013,1019,1021,1031,1033,1039,1049,1051,1061,1063,1069,1087,1091,1093,1097,1103,1109,1117,1123,1129,1151,1153,1163,1171,1181,1187,1193,1201,1213,1217,1223,1229,1231,1237,1249,1259,1277,1279,1283,1289,1291,1297,1301,1303,1307,1319,1321,1327,1361,1367,1373,1381,1399,1409,1423,1427,1429,1433,1439,1447,1451,1453,1459,1471,1481,1483,1487,1489,1493,1499,1511,1523,1531,1543,1549,1553,1559,1567,1571,1579,1583,1597,1601,1607,1609,1613,1619,1621,1627,1637,1657,1663,1667,1669,1693,1697,1699,1709,1721,1723,1733,1741,1747,1753,1759,1777,1783,1787,1789,1801,1811,1823,1831,1847,1861,1867,1871,1873,1877,1879,1889,1901,1907,1913,1931,1933,1949,1951,1973,1979,1987,1993,1997,1999,2003,2011,2017,2027,2029,2039,2053,2063,2069,2081,2083,2087,2089,2099,2111,2113,2129,2131,2137,2141,2143,2153,2161,2179,2203,2207,2213,2221,2237,2239,2243,2251,2267,2269,2273,2281,2287,2293,2297,2309,2311,2333,2339,2341,2347,2351,2357,2371,2377,2381,2383,2389,2393,2399,2411,2417,2423,2437,2441,2447,2459,2467,2473,2477,2503,2521,2531,2539,2543,2549,2551,2557,2579,2591,2593,2609,2617,2621,2633,2647,2657,2659,2663,2671,2677,2683,2687,2689,2693,2699,2707,2711,2713,2719,2729,2731,2741,2749,2753,2767,2777,2789,2791,2797,2801,2803,2819,2833,2837,2843,2851,2857,2861,2879,2887,2897,2903,2909,2917,2927,2939,2953,2957,2963,2969,2971,2999,3001,3011,3019,3023,3037,3041,3049,3061,3067,3079,3083,3089,3109,3119,3121,3137,3163,3167,3169,3181,3187,3191,3203,3209,3217,3221,3229,3251,3253,3257,3259,3271,3299,3301,3307,3313,3319,3323,3329,3331,3343,3347,3359,3361,3371,3373,3389,3391,3407,3413,3433,3449,3457,3461,3463,3467,3469,3491,3499,3511,3517,3527,3529,3533,3539,3541,3547,3557,3559,3571,3581,3583,3593,3607,3613,3617,3623,3631,3637,3643,3659,3671,3673,3677,3691,3697,3701,3709,3719,3727,3733,3739,3761,3767,3769,3779,3793,3797,3803,3821,3823,3833,3847,3851,3853,3863,3877,3881,3889,3907,3911,3917,3919,3923,3929,3931,3943,3947,3967,3989,4001,4003,4007,4013,4019,4021,4027,4049,4051,4057,4073,4079,4091,4093,4099,4111,4127,4129,4133,4139,4153,4157,4159,4177,4201,4211,4217,4219,4229,4231,4241,4243,4253,4259,4261,4271,4273,4283,4289,4297,4327,4337,4339,4349,4357,4363,4373,4391,4397,4409,4421,4423,4441,4447,4451,4457,4463,4481,4483,4493,4507,4513,4517,4519,4523,4547,4549,4561,4567,4583,4591,4597,4603,4621,4637,4639,4643,4649,4651,4657,4663,4673,4679,4691,4703,4721,4723,4729,4733,4751,4759,4783,4787,4789,4793,4799,4801,4813,4817,4831,4861,4871,4877,4889,4903,4909,4919,4931,4933,4937,4943,4951,4957,4967,4969,4973,4987,4993,4999,5003,5009,5011,5021,5023,5039,5051,5059,5077,5081,5087,5099,5101,5107,5113,5119,5147,5153,5167,5171,5179,5189,5197,5209,5227,5231,5233,5237,5261,5273,5279,5281,5297,5303,5309,5323,5333,5347,5351,5381,5387,5393,5399,5407,5413,5417,5419,5431,5437,5441,5443,5449,5471,5477,5479,5483,5501,5503,5507,5519,5521,5527,5531,5557,5563,5569,5573,5581,5591,5623,5639,5641,5647,5651,5653,5657,5659,5669,5683,5689,5693,5701,5711,5717,5737,5741,5743,5749,5779,5783,5791,5801,5807,5813,5821,5827,5839,5843,5849,5851,5857,5861,5867,5869,5879,5881,5897,5903,5923,5927,5939,5953,5981,5987,6007,6011,6029,6037,6043,6047,6053,6067,6073,6079,6089,6091,6101,6113,6121,6131,6133,6143,6151,6163,6173,6197,6199,6203,6211,6217,6221,6229,6247,6257,6263,6269,6271,6277,6287,6299,6301,6311,6317,6323,6329,6337,6343,6353,6359,6361,6367,6373,6379,6389,6397,6421,6427,6449,6451,6469,6473,6481,6491,6521,6529,6547,6551,6553,6563,6569,6571,6577,6581,6599,6607,6619,6637,6653,6659,6661,6673,6679,6689,6691,6701,6703,6709,6719,6733,6737,6761,6763,6779,6781,6791,6793,6803,6823,6827,6829,6833,6841,6857,6863,6869,6871,6883,6899,6907,6911,6917,6947,6949,6959,6961,6967,6971,6977,6983,6991,6997,7001,7013,7019,7027,7039,7043,7057,7069,7079,7103,7109,7121,7127,7129,7151,7159,7177,7187,7193,7207,7211,7213,7219,7229,7237,7243,7247,7253,7283,7297,7307,7309,7321,7331,7333,7349,7351,7369,7393,7411,7417,7433,7451,7457,7459,7477,7481,7487,7489,7499,7507,7517,7523,7529,7537,7541,7547,7549,7559,7561,7573,7577,7583,7589,7591,7603,7607,7621,7639,7643,7649,7669,7673,7681,7687,7691,7699,7703,7717,7723,7727,7741,7753,7757,7759,7789,7793,7817,7823,7829,7841,7853,7867,7873,7877,7879,7883,7901,7907,7919,7927,7933,7937,7949,7951,7963,7993,8009,8011,8017,8039,8053,8059,8069,8081,8087,8089,8093,8101,8111,8117,8123,8147,8161,8167,8171,8179,8191,8209,8219,8221,8231,8233,8237,8243,8263,8269,8273,8287,8291,8293,8297,8311,8317,8329,8353,8363,8369,8377,8387,8389,8419,8423,8429,8431,8443,8447,8461,8467,8501,8513,8521,8527,8537,8539,8543,8563,8573,8581,8597,8599,8609,8623,8627,8629,8641,8647,8663,8669,8677,8681,8689,8693,8699,8707,8713,8719,8731,8737,8741,8747,8753,8761,8779,8783,8803,8807,8819,8821,8831,8837,8839,8849,8861,8863,8867,8887,8893,8923,8929,8933,8941,8951,8963,8969,8971,8999,9001,9007,9011,9013,9029,9041,9043,9049,9059,9067,9091,9103,9109,9127,9133,9137,9151,9157,9161,9173,9181,9187,9199,9203,9209,9221,9227,9239,9241,9257,9277,9281,9283,9293,9311,9319,9323,9337,9341,9343,9349,9371,9377,9391,9397,9403,9413,9419,9421,9431,9433,9437,9439,9461,9463,9467,9473,9479,9491,9497,9511,9521,9533,9539,9547,9551,9587,9601,9613,9619,9623,9629,9631,9643,9649,9661,9677,9679,9689,9697,9719,9721,9733,9739,9743,9749,9767,9769,9781,9787,9791,9803,9811,9817,9829,9833,9839,9851,9857,9859,9871,9883,9887,9901,9907,9923,9929,9931,9941,9949,9967,9973,10007,10009,10037,10039,10061,10067,10069,10079,10091,10093,10099,10103,10111,10133,10139,10141,10151,10159,10163,10169,10177,10181,10193,10211,10223,10243,10247,10253,10259,10267,10271,10273,10289,10301,10303,10313,10321,10331,10333,10337,10343,10357,10369,10391,10399,10427,10429,10433,10453,10457,10459,10463,10477,10487,10499,10501,10513,10529,10531,10559,10567,10589,10597,10601,10607,10613,10627,10631,10639,10651,10657,10663,10667,10687,10691,10709,10711,10723,10729,10733,10739,10753,10771,10781,10789,10799,10831,10837,10847,10853,10859,10861,10867,10883,10889,10891,10903,10909,10937,10939,10949,10957,10973,10979,10987,10993,11003,11027,11047,11057,11059,11069,11071,11083,11087,11093,11113,11117,11119,11131,11149,11159,11161,11171,11173,11177,11197,11213,11239,11243,11251,11257,11261,11273,11279,11287,11299,11311,11317,11321,11329,11351,11353,11369,11383,11393,11399,11411,11423,11437,11443,11447,11467,11471,11483,11489,11491,11497,11503,11519,11527,11549,11551,11579,11587,11593,11597,11617,11621,11633,11657,11677,11681,11689,11699,11701,11717,11719,11731,11743,11777,11779,11783,11789,11801,11807,11813,11821,11827,11831,11833,11839,11863,11867,11887,11897,11903,11909,11923,11927,11933,11939,11941,11953,11959,11969,11971,11981,11987,12007,12011,12037,12041,12043,12049,12071,12073,12097,12101,12107,12109,12113,12119,12143,12149,12157,12161,12163,12197,12203,12211,12227,12239,12241,12251,12253,12263,12269,12277,12281,12289,12301,12323,12329,12343,12347,12373,12377,12379,12391,12401,12409,12413,12421,12433,12437,12451,12457,12473,12479,12487,12491,12497,12503,12511,12517,12527,12539,12541,12547,12553,12569,12577,12583,12589,12601,12611,12613,12619,12637,12641,12647,12653,12659,12671,12689,12697,12703,12713,12721,12739,12743,12757,12763,12781,12791,12799,12809,12821,12823,12829,12841,12853,12889,12893,12899,12907,12911,12917,12919,12923,12941,12953,12959,12967,12973,12979,12983,13001,13003,13007,13009,13033,13037,13043,13049,13063,13093,13099,13103,13109,13121,13127,13147,13151,13159,13163,13171,13177,13183,13187,13217,13219,13229,13241,13249,13259,13267,13291,13297,13309,13313,13327,13331,13337,13339,13367,13381,13397,13399,13411,13417,13421,13441,13451,13457,13463,13469,13477,13487,13499,13513,13523,13537,13553,13567,13577,13591,13597,13613,13619,13627,13633,13649,13669,13679,13681,13687,13691,13693,13697,13709,13711,13721,13723,13729,13751,13757,13759,13763,13781,13789,13799,13807,13829,13831,13841,13859,13873,13877,13879,13883,13901,13903,13907,13913,13921,13931,13933,13963,13967,13997,13999,14009,14011,14029,14033,14051,14057,14071,14081,14083,14087,14107,14143,14149,14153,14159,14173,14177,14197,14207,14221,14243,14249,14251,14281,14293,14303,14321,14323,14327,14341,14347,14369,14387,14389,14401,14407,14411,14419,14423,14431,14437,14447,14449,14461,14479,14489,14503,14519,14533,14537,14543,14549,14551,14557,14561,14563,14591,14593,14621,14627,14629,14633,14639,14653,14657,14669,14683,14699,14713,14717,14723,14731,14737,14741,14747,14753,14759,14767,14771,14779,14783,14797,14813,14821,14827,14831,14843,14851,14867,14869,14879,14887,14891,14897,14923,14929,14939,14947,14951,14957,14969,14983,15013,15017,15031,15053,15061,15073,15077,15083,15091,15101,15107,15121,15131,15137,15139,15149,15161,15173,15187,15193,15199,15217,15227,15233,15241,15259,15263,15269,15271,15277,15287,15289,15299,15307,15313,15319,15329,15331,15349,15359,15361,15373,15377,15383,15391,15401,15413,15427,15439,15443,15451,15461,15467,15473,15493,15497,15511,15527,15541,15551,15559,15569,15581,15583,15601,15607,15619,15629,15641,15643,15647,15649,15661,15667,15671,15679,15683,15727,15731,15733,15737,15739,15749,15761,15767,15773,15787,15791,15797,15803,15809,15817,15823,15859,15877,15881,15887,15889,15901,15907,15913,15919,15923,15937,15959,15971,15973,15991,16001,16007,16033,16057,16061,16063,16067,16069,16073,16087,16091,16097,16103,16111,16127,16139,16141,16183,16187,16189,16193,16217,16223,16229,16231,16249,16253,16267,16273,16301,16319,16333,16339,16349,16361,16363,16369,16381,16411,16417,16421,16427,16433,16447,16451,16453,16477,16481,16487,16493,16519,16529,16547,16553,16561,16567,16573,16603,16607,16619,16631,16633,16649,16651,16657,16661,16673,16691,16693,16699,16703,16729,16741,16747,16759,16763,16787,16811,16823,16829,16831,16843,16871,16879,16883,16889,16901,16903,16921,16927,16931,16937,16943,16963,16979,16981,16987,16993,17011,17021,17027,17029,17033,17041,17047,17053,17077,17093,17099,17107,17117,17123,17137,17159,17167,17183,17189,17191,17203,17207,17209,17231,17239,17257,17291,17293,17299,17317,17321,17327,17333,17341,17351,17359,17377,17383,17387,17389,17393,17401,17417,17419,17431,17443,17449,17467,17471,17477,17483,17489,17491,17497,17509,17519,17539,17551,17569,17573,17579,17581,17597,17599,17609,17623,17627,17657,17659,17669,17681,17683,17707,17713,17729,17737,17747,17749,17761,17783,17789,17791,17807,17827,17837,17839,17851,17863};
struct _reverb_faustSIG0 {
	int i_reverb_faustSIG0Wave0_idx;
	
	int getNumInputs_reverb_faustSIG0() {
		return 0;
	}
	int getNumOutputs_reverb_faustSIG0() {
		return 1;
	}
	
	void instanceInit_reverb_faustSIG0(int sample_rate) {
		i_reverb_faustSIG0Wave0_idx = 0;
	}
	
	void fill_reverb_faustSIG0(int count, int* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			table[i1] = i_reverb_faustSIG0Wave0[i_reverb_faustSIG0Wave0_idx];
			i_reverb_faustSIG0Wave0_idx = (1 + i_reverb_faustSIG0Wave0_idx) % 2048;
		}
	}

};

static _reverb_faustSIG0* new_reverb_faustSIG0() { return (_reverb_faustSIG0*)new _reverb_faustSIG0(); }
static void delete_reverb_faustSIG0(_reverb_faustSIG0* dsp) { delete dsp; }

static int itbl0_reverb_faustSIG0[2048];

struct _reverb_faust final : public ::faust::dsp {
	
	int iVec0[2];
	FAUSTFLOAT fHslider0;
	float fVec1[2];
	FAUSTFLOAT fHslider1;
	float fVec2[2];
	int IOTA0;
	FAUSTFLOAT fHslider2;
	float fVec3[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec3[2];
	float fRec4[2];
	FAUSTFLOAT fHslider3;
	float fVec4[2];
	float fConst2;
	float fVec5[131072];
	FAUSTFLOAT fHslider4;
	float fRec5[2];
	float fRec6[2];
	float fRec7[2];
	float fRec8[2];
	FAUSTFLOAT fHslider5;
	float fVec6[2];
	float fVec7[131072];
	FAUSTFLOAT fHslider6;
	float fRec21[2];
	float fVec8[16384];
	float fVec9[2];
	float fRec20[2];
	float fRec18[2];
	float fRec23[2];
	float fVec10[16384];
	float fVec11[2];
	float fRec22[2];
	float fRec19[2];
	float fVec12[16384];
	float fRec24[2];
	float fVec13[2];
	float fRec17[2];
	float fRec15[2];
	float fRec26[2];
	float fVec14[16384];
	float fVec15[2];
	float fRec25[2];
	float fRec16[2];
	float fVec16[16384];
	float fRec27[2];
	float fVec17[2];
	float fRec14[2];
	float fRec12[2];
	float fRec29[2];
	float fVec18[16384];
	float fVec19[2];
	float fRec28[2];
	float fRec13[2];
	float fVec20[16384];
	float fRec30[2];
	float fVec21[2];
	float fRec11[2];
	float fRec9[2];
	float fRec32[2];
	float fVec22[16384];
	float fVec23[2];
	float fRec31[2];
	float fRec10[2];
	float fRec45[2];
	float fVec24[16384];
	float fVec25[2];
	float fRec44[2];
	float fRec42[2];
	float fRec47[2];
	float fVec26[16384];
	float fVec27[2];
	float fRec46[2];
	float fRec43[2];
	float fVec28[16384];
	float fRec48[2];
	float fVec29[2];
	float fRec41[2];
	float fRec39[2];
	float fRec50[2];
	float fVec30[16384];
	float fVec31[2];
	float fRec49[2];
	float fRec40[2];
	float fVec32[16384];
	float fRec51[2];
	float fVec33[2];
	float fRec38[2];
	float fRec36[2];
	float fRec53[2];
	float fVec34[16384];
	float fVec35[2];
	float fRec52[2];
	float fRec37[2];
	float fVec36[16384];
	float fRec54[2];
	float fVec37[2];
	float fRec35[2];
	float fRec33[2];
	float fRec56[2];
	float fVec38[16384];
	float fVec39[2];
	float fRec55[2];
	float fRec34[2];
	float fRec69[2];
	float fVec40[16384];
	float fVec41[2];
	float fRec68[2];
	float fRec66[2];
	float fRec71[2];
	float fVec42[16384];
	float fVec43[2];
	float fRec70[2];
	float fRec67[2];
	float fVec44[16384];
	float fRec72[2];
	float fVec45[2];
	float fRec65[2];
	float fRec63[2];
	float fRec74[2];
	float fVec46[16384];
	float fVec47[2];
	float fRec73[2];
	float fRec64[2];
	float fVec48[16384];
	float fRec75[2];
	float fVec49[2];
	float fRec62[2];
	float fRec60[2];
	float fRec77[2];
	float fVec50[16384];
	float fVec51[2];
	float fRec76[2];
	float fRec61[2];
	float fVec52[16384];
	float fRec78[2];
	float fVec53[2];
	float fRec59[2];
	float fRec57[2];
	float fRec80[2];
	float fVec54[16384];
	float fVec55[2];
	float fRec79[2];
	float fRec58[2];
	float fRec2[2];
	float fRec0[1024];
	float fRec81[2];
	float fRec1[1024];
	
	_reverb_faust() {
	}
	
	void metadata(Meta* m) { 
		m->declare("author", "GRAME");
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/version", "1.21.0");
		m->declare("compile_options", "-lang cpp -rui -nvi -ct 1 -cn _reverb_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0");
		m->declare("delays.lib/fdelay1a:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelay4:author", "Julius O. Smith III");
		m->declare("delays.lib/fdelayltv:author", "Julius O. Smith III");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.2.0");
		m->declare("demos.lib/greyhole_demo:author", "Till Bovermann");
		m->declare("demos.lib/greyhole_demo:license", "GPL2+");
		m->declare("demos.lib/name", "Faust Demos Library");
		m->declare("demos.lib/version", "1.2.0");
		m->declare("description", "Greyhole demo application.");
		m->declare("filename", "reverb_faust.dsp");
		m->declare("filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
		m->declare("filters.lib/name", "Faust Filters Library");
		m->declare("filters.lib/nlf2:author", "Julius O. Smith III");
		m->declare("filters.lib/nlf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/nlf2:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/tf1:author", "Julius O. Smith III");
		m->declare("filters.lib/tf1:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
		m->declare("filters.lib/tf1:license", "MIT-style STK-4.3 license");
		m->declare("filters.lib/version", "1.7.1");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.8.1");
		m->declare("name", "reverb_faust");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.6.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("reverbs.lib/greyhole:author", "Julian Parker, bug fixes and minor interface changes by Till Bovermann");
		m->declare("reverbs.lib/greyhole:license", "MIT license");
		m->declare("reverbs.lib/name", "Faust Reverb Library");
		m->declare("reverbs.lib/version", "1.4.0");
		m->declare("signals.lib/name", "Faust Signal Routing Library");
		m->declare("signals.lib/version", "1.6.0");
		m->declare("version", "0.1");
	}

	static constexpr int getStaticNumInputs() {
		return 2;
	}
	static constexpr int getStaticNumOutputs() {
		return 2;
	}
	int getNumInputs() {
		return 2;
	}
	int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
		_reverb_faustSIG0* sig0 = new_reverb_faustSIG0();
		sig0->instanceInit_reverb_faustSIG0(sample_rate);
		sig0->fill_reverb_faustSIG0(2048, itbl0_reverb_faustSIG0);
		delete_reverb_faustSIG0(sig0);
	}
	
	void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
		fConst1 = 3.1415927f / fConst0;
		fConst2 = 0.00056689343f * fConst0;
	}
	
	void instanceResetUserInterface() {
		fHslider0 = FAUSTFLOAT(0.0f);
		fHslider1 = FAUSTFLOAT(0.9f);
		fHslider2 = FAUSTFLOAT(2.0f);
		fHslider3 = FAUSTFLOAT(0.1f);
		fHslider4 = FAUSTFLOAT(0.2f);
		fHslider5 = FAUSTFLOAT(0.5f);
		fHslider6 = FAUSTFLOAT(1.0f);
	}
	
	void instanceClear() {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fVec1[l1] = 0.0f;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fVec2[l2] = 0.0f;
		}
		IOTA0 = 0;
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fVec3[l3] = 0.0f;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec3[l4] = 0.0f;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec4[l5] = 0.0f;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fVec4[l6] = 0.0f;
		}
		for (int l7 = 0; l7 < 131072; l7 = l7 + 1) {
			fVec5[l7] = 0.0f;
		}
		for (int l8 = 0; l8 < 2; l8 = l8 + 1) {
			fRec5[l8] = 0.0f;
		}
		for (int l9 = 0; l9 < 2; l9 = l9 + 1) {
			fRec6[l9] = 0.0f;
		}
		for (int l10 = 0; l10 < 2; l10 = l10 + 1) {
			fRec7[l10] = 0.0f;
		}
		for (int l11 = 0; l11 < 2; l11 = l11 + 1) {
			fRec8[l11] = 0.0f;
		}
		for (int l12 = 0; l12 < 2; l12 = l12 + 1) {
			fVec6[l12] = 0.0f;
		}
		for (int l13 = 0; l13 < 131072; l13 = l13 + 1) {
			fVec7[l13] = 0.0f;
		}
		for (int l14 = 0; l14 < 2; l14 = l14 + 1) {
			fRec21[l14] = 0.0f;
		}
		for (int l15 = 0; l15 < 16384; l15 = l15 + 1) {
			fVec8[l15] = 0.0f;
		}
		for (int l16 = 0; l16 < 2; l16 = l16 + 1) {
			fVec9[l16] = 0.0f;
		}
		for (int l17 = 0; l17 < 2; l17 = l17 + 1) {
			fRec20[l17] = 0.0f;
		}
		for (int l18 = 0; l18 < 2; l18 = l18 + 1) {
			fRec18[l18] = 0.0f;
		}
		for (int l19 = 0; l19 < 2; l19 = l19 + 1) {
			fRec23[l19] = 0.0f;
		}
		for (int l20 = 0; l20 < 16384; l20 = l20 + 1) {
			fVec10[l20] = 0.0f;
		}
		for (int l21 = 0; l21 < 2; l21 = l21 + 1) {
			fVec11[l21] = 0.0f;
		}
		for (int l22 = 0; l22 < 2; l22 = l22 + 1) {
			fRec22[l22] = 0.0f;
		}
		for (int l23 = 0; l23 < 2; l23 = l23 + 1) {
			fRec19[l23] = 0.0f;
		}
		for (int l24 = 0; l24 < 16384; l24 = l24 + 1) {
			fVec12[l24] = 0.0f;
		}
		for (int l25 = 0; l25 < 2; l25 = l25 + 1) {
			fRec24[l25] = 0.0f;
		}
		for (int l26 = 0; l26 < 2; l26 = l26 + 1) {
			fVec13[l26] = 0.0f;
		}
		for (int l27 = 0; l27 < 2; l27 = l27 + 1) {
			fRec17[l27] = 0.0f;
		}
		for (int l28 = 0; l28 < 2; l28 = l28 + 1) {
			fRec15[l28] = 0.0f;
		}
		for (int l29 = 0; l29 < 2; l29 = l29 + 1) {
			fRec26[l29] = 0.0f;
		}
		for (int l30 = 0; l30 < 16384; l30 = l30 + 1) {
			fVec14[l30] = 0.0f;
		}
		for (int l31 = 0; l31 < 2; l31 = l31 + 1) {
			fVec15[l31] = 0.0f;
		}
		for (int l32 = 0; l32 < 2; l32 = l32 + 1) {
			fRec25[l32] = 0.0f;
		}
		for (int l33 = 0; l33 < 2; l33 = l33 + 1) {
			fRec16[l33] = 0.0f;
		}
		for (int l34 = 0; l34 < 16384; l34 = l34 + 1) {
			fVec16[l34] = 0.0f;
		}
		for (int l35 = 0; l35 < 2; l35 = l35 + 1) {
			fRec27[l35] = 0.0f;
		}
		for (int l36 = 0; l36 < 2; l36 = l36 + 1) {
			fVec17[l36] = 0.0f;
		}
		for (int l37 = 0; l37 < 2; l37 = l37 + 1) {
			fRec14[l37] = 0.0f;
		}
		for (int l38 = 0; l38 < 2; l38 = l38 + 1) {
			fRec12[l38] = 0.0f;
		}
		for (int l39 = 0; l39 < 2; l39 = l39 + 1) {
			fRec29[l39] = 0.0f;
		}
		for (int l40 = 0; l40 < 16384; l40 = l40 + 1) {
			fVec18[l40] = 0.0f;
		}
		for (int l41 = 0; l41 < 2; l41 = l41 + 1) {
			fVec19[l41] = 0.0f;
		}
		for (int l42 = 0; l42 < 2; l42 = l42 + 1) {
			fRec28[l42] = 0.0f;
		}
		for (int l43 = 0; l43 < 2; l43 = l43 + 1) {
			fRec13[l43] = 0.0f;
		}
		for (int l44 = 0; l44 < 16384; l44 = l44 + 1) {
			fVec20[l44] = 0.0f;
		}
		for (int l45 = 0; l45 < 2; l45 = l45 + 1) {
			fRec30[l45] = 0.0f;
		}
		for (int l46 = 0; l46 < 2; l46 = l46 + 1) {
			fVec21[l46] = 0.0f;
		}
		for (int l47 = 0; l47 < 2; l47 = l47 + 1) {
			fRec11[l47] = 0.0f;
		}
		for (int l48 = 0; l48 < 2; l48 = l48 + 1) {
			fRec9[l48] = 0.0f;
		}
		for (int l49 = 0; l49 < 2; l49 = l49 + 1) {
			fRec32[l49] = 0.0f;
		}
		for (int l50 = 0; l50 < 16384; l50 = l50 + 1) {
			fVec22[l50] = 0.0f;
		}
		for (int l51 = 0; l51 < 2; l51 = l51 + 1) {
			fVec23[l51] = 0.0f;
		}
		for (int l52 = 0; l52 < 2; l52 = l52 + 1) {
			fRec31[l52] = 0.0f;
		}
		for (int l53 = 0; l53 < 2; l53 = l53 + 1) {
			fRec10[l53] = 0.0f;
		}
		for (int l54 = 0; l54 < 2; l54 = l54 + 1) {
			fRec45[l54] = 0.0f;
		}
		for (int l55 = 0; l55 < 16384; l55 = l55 + 1) {
			fVec24[l55] = 0.0f;
		}
		for (int l56 = 0; l56 < 2; l56 = l56 + 1) {
			fVec25[l56] = 0.0f;
		}
		for (int l57 = 0; l57 < 2; l57 = l57 + 1) {
			fRec44[l57] = 0.0f;
		}
		for (int l58 = 0; l58 < 2; l58 = l58 + 1) {
			fRec42[l58] = 0.0f;
		}
		for (int l59 = 0; l59 < 2; l59 = l59 + 1) {
			fRec47[l59] = 0.0f;
		}
		for (int l60 = 0; l60 < 16384; l60 = l60 + 1) {
			fVec26[l60] = 0.0f;
		}
		for (int l61 = 0; l61 < 2; l61 = l61 + 1) {
			fVec27[l61] = 0.0f;
		}
		for (int l62 = 0; l62 < 2; l62 = l62 + 1) {
			fRec46[l62] = 0.0f;
		}
		for (int l63 = 0; l63 < 2; l63 = l63 + 1) {
			fRec43[l63] = 0.0f;
		}
		for (int l64 = 0; l64 < 16384; l64 = l64 + 1) {
			fVec28[l64] = 0.0f;
		}
		for (int l65 = 0; l65 < 2; l65 = l65 + 1) {
			fRec48[l65] = 0.0f;
		}
		for (int l66 = 0; l66 < 2; l66 = l66 + 1) {
			fVec29[l66] = 0.0f;
		}
		for (int l67 = 0; l67 < 2; l67 = l67 + 1) {
			fRec41[l67] = 0.0f;
		}
		for (int l68 = 0; l68 < 2; l68 = l68 + 1) {
			fRec39[l68] = 0.0f;
		}
		for (int l69 = 0; l69 < 2; l69 = l69 + 1) {
			fRec50[l69] = 0.0f;
		}
		for (int l70 = 0; l70 < 16384; l70 = l70 + 1) {
			fVec30[l70] = 0.0f;
		}
		for (int l71 = 0; l71 < 2; l71 = l71 + 1) {
			fVec31[l71] = 0.0f;
		}
		for (int l72 = 0; l72 < 2; l72 = l72 + 1) {
			fRec49[l72] = 0.0f;
		}
		for (int l73 = 0; l73 < 2; l73 = l73 + 1) {
			fRec40[l73] = 0.0f;
		}
		for (int l74 = 0; l74 < 16384; l74 = l74 + 1) {
			fVec32[l74] = 0.0f;
		}
		for (int l75 = 0; l75 < 2; l75 = l75 + 1) {
			fRec51[l75] = 0.0f;
		}
		for (int l76 = 0; l76 < 2; l76 = l76 + 1) {
			fVec33[l76] = 0.0f;
		}
		for (int l77 = 0; l77 < 2; l77 = l77 + 1) {
			fRec38[l77] = 0.0f;
		}
		for (int l78 = 0; l78 < 2; l78 = l78 + 1) {
			fRec36[l78] = 0.0f;
		}
		for (int l79 = 0; l79 < 2; l79 = l79 + 1) {
			fRec53[l79] = 0.0f;
		}
		for (int l80 = 0; l80 < 16384; l80 = l80 + 1) {
			fVec34[l80] = 0.0f;
		}
		for (int l81 = 0; l81 < 2; l81 = l81 + 1) {
			fVec35[l81] = 0.0f;
		}
		for (int l82 = 0; l82 < 2; l82 = l82 + 1) {
			fRec52[l82] = 0.0f;
		}
		for (int l83 = 0; l83 < 2; l83 = l83 + 1) {
			fRec37[l83] = 0.0f;
		}
		for (int l84 = 0; l84 < 16384; l84 = l84 + 1) {
			fVec36[l84] = 0.0f;
		}
		for (int l85 = 0; l85 < 2; l85 = l85 + 1) {
			fRec54[l85] = 0.0f;
		}
		for (int l86 = 0; l86 < 2; l86 = l86 + 1) {
			fVec37[l86] = 0.0f;
		}
		for (int l87 = 0; l87 < 2; l87 = l87 + 1) {
			fRec35[l87] = 0.0f;
		}
		for (int l88 = 0; l88 < 2; l88 = l88 + 1) {
			fRec33[l88] = 0.0f;
		}
		for (int l89 = 0; l89 < 2; l89 = l89 + 1) {
			fRec56[l89] = 0.0f;
		}
		for (int l90 = 0; l90 < 16384; l90 = l90 + 1) {
			fVec38[l90] = 0.0f;
		}
		for (int l91 = 0; l91 < 2; l91 = l91 + 1) {
			fVec39[l91] = 0.0f;
		}
		for (int l92 = 0; l92 < 2; l92 = l92 + 1) {
			fRec55[l92] = 0.0f;
		}
		for (int l93 = 0; l93 < 2; l93 = l93 + 1) {
			fRec34[l93] = 0.0f;
		}
		for (int l94 = 0; l94 < 2; l94 = l94 + 1) {
			fRec69[l94] = 0.0f;
		}
		for (int l95 = 0; l95 < 16384; l95 = l95 + 1) {
			fVec40[l95] = 0.0f;
		}
		for (int l96 = 0; l96 < 2; l96 = l96 + 1) {
			fVec41[l96] = 0.0f;
		}
		for (int l97 = 0; l97 < 2; l97 = l97 + 1) {
			fRec68[l97] = 0.0f;
		}
		for (int l98 = 0; l98 < 2; l98 = l98 + 1) {
			fRec66[l98] = 0.0f;
		}
		for (int l99 = 0; l99 < 2; l99 = l99 + 1) {
			fRec71[l99] = 0.0f;
		}
		for (int l100 = 0; l100 < 16384; l100 = l100 + 1) {
			fVec42[l100] = 0.0f;
		}
		for (int l101 = 0; l101 < 2; l101 = l101 + 1) {
			fVec43[l101] = 0.0f;
		}
		for (int l102 = 0; l102 < 2; l102 = l102 + 1) {
			fRec70[l102] = 0.0f;
		}
		for (int l103 = 0; l103 < 2; l103 = l103 + 1) {
			fRec67[l103] = 0.0f;
		}
		for (int l104 = 0; l104 < 16384; l104 = l104 + 1) {
			fVec44[l104] = 0.0f;
		}
		for (int l105 = 0; l105 < 2; l105 = l105 + 1) {
			fRec72[l105] = 0.0f;
		}
		for (int l106 = 0; l106 < 2; l106 = l106 + 1) {
			fVec45[l106] = 0.0f;
		}
		for (int l107 = 0; l107 < 2; l107 = l107 + 1) {
			fRec65[l107] = 0.0f;
		}
		for (int l108 = 0; l108 < 2; l108 = l108 + 1) {
			fRec63[l108] = 0.0f;
		}
		for (int l109 = 0; l109 < 2; l109 = l109 + 1) {
			fRec74[l109] = 0.0f;
		}
		for (int l110 = 0; l110 < 16384; l110 = l110 + 1) {
			fVec46[l110] = 0.0f;
		}
		for (int l111 = 0; l111 < 2; l111 = l111 + 1) {
			fVec47[l111] = 0.0f;
		}
		for (int l112 = 0; l112 < 2; l112 = l112 + 1) {
			fRec73[l112] = 0.0f;
		}
		for (int l113 = 0; l113 < 2; l113 = l113 + 1) {
			fRec64[l113] = 0.0f;
		}
		for (int l114 = 0; l114 < 16384; l114 = l114 + 1) {
			fVec48[l114] = 0.0f;
		}
		for (int l115 = 0; l115 < 2; l115 = l115 + 1) {
			fRec75[l115] = 0.0f;
		}
		for (int l116 = 0; l116 < 2; l116 = l116 + 1) {
			fVec49[l116] = 0.0f;
		}
		for (int l117 = 0; l117 < 2; l117 = l117 + 1) {
			fRec62[l117] = 0.0f;
		}
		for (int l118 = 0; l118 < 2; l118 = l118 + 1) {
			fRec60[l118] = 0.0f;
		}
		for (int l119 = 0; l119 < 2; l119 = l119 + 1) {
			fRec77[l119] = 0.0f;
		}
		for (int l120 = 0; l120 < 16384; l120 = l120 + 1) {
			fVec50[l120] = 0.0f;
		}
		for (int l121 = 0; l121 < 2; l121 = l121 + 1) {
			fVec51[l121] = 0.0f;
		}
		for (int l122 = 0; l122 < 2; l122 = l122 + 1) {
			fRec76[l122] = 0.0f;
		}
		for (int l123 = 0; l123 < 2; l123 = l123 + 1) {
			fRec61[l123] = 0.0f;
		}
		for (int l124 = 0; l124 < 16384; l124 = l124 + 1) {
			fVec52[l124] = 0.0f;
		}
		for (int l125 = 0; l125 < 2; l125 = l125 + 1) {
			fRec78[l125] = 0.0f;
		}
		for (int l126 = 0; l126 < 2; l126 = l126 + 1) {
			fVec53[l126] = 0.0f;
		}
		for (int l127 = 0; l127 < 2; l127 = l127 + 1) {
			fRec59[l127] = 0.0f;
		}
		for (int l128 = 0; l128 < 2; l128 = l128 + 1) {
			fRec57[l128] = 0.0f;
		}
		for (int l129 = 0; l129 < 2; l129 = l129 + 1) {
			fRec80[l129] = 0.0f;
		}
		for (int l130 = 0; l130 < 16384; l130 = l130 + 1) {
			fVec54[l130] = 0.0f;
		}
		for (int l131 = 0; l131 < 2; l131 = l131 + 1) {
			fVec55[l131] = 0.0f;
		}
		for (int l132 = 0; l132 < 2; l132 = l132 + 1) {
			fRec79[l132] = 0.0f;
		}
		for (int l133 = 0; l133 < 2; l133 = l133 + 1) {
			fRec58[l133] = 0.0f;
		}
		for (int l134 = 0; l134 < 2; l134 = l134 + 1) {
			fRec2[l134] = 0.0f;
		}
		for (int l135 = 0; l135 < 1024; l135 = l135 + 1) {
			fRec0[l135] = 0.0f;
		}
		for (int l136 = 0; l136 < 2; l136 = l136 + 1) {
			fRec81[l136] = 0.0f;
		}
		for (int l137 = 0; l137 < 1024; l137 = l137 + 1) {
			fRec1[l137] = 0.0f;
		}
	}
	
	void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	_reverb_faust* clone() {
		return new _reverb_faust();
	}
	
	int getSampleRate() {
		return fSampleRate;
	}
	
	void buildUserInterface(UI* ui_interface) {
		ui_interface->declare(0, "0", "");
		ui_interface->openVerticalBox("Greyhole");
		ui_interface->declare(0, "0", "");
		ui_interface->openHorizontalBox("Mix");
		ui_interface->declare(&fHslider4, "01", "");
		ui_interface->declare(&fHslider4, "style", "knob");
		ui_interface->addHorizontalSlider("delayTime", &fHslider4, FAUSTFLOAT(0.2f), FAUSTFLOAT(0.001f), FAUSTFLOAT(1.45f), FAUSTFLOAT(0.0001f));
		ui_interface->declare(&fHslider0, "02", "");
		ui_interface->declare(&fHslider0, "style", "knob");
		ui_interface->addHorizontalSlider("damping", &fHslider0, FAUSTFLOAT(0.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.99f), FAUSTFLOAT(0.001f));
		ui_interface->declare(&fHslider6, "03", "");
		ui_interface->declare(&fHslider6, "style", "knob");
		ui_interface->addHorizontalSlider("size", &fHslider6, FAUSTFLOAT(1.0f), FAUSTFLOAT(0.5f), FAUSTFLOAT(3.0f), FAUSTFLOAT(0.0001f));
		ui_interface->declare(&fHslider5, "04", "");
		ui_interface->declare(&fHslider5, "style", "knob");
		ui_interface->addHorizontalSlider("diffusion", &fHslider5, FAUSTFLOAT(0.5f), FAUSTFLOAT(0.0f), FAUSTFLOAT(0.99f), FAUSTFLOAT(0.0001f));
		ui_interface->declare(&fHslider1, "05", "");
		ui_interface->declare(&fHslider1, "style", "knob");
		ui_interface->addHorizontalSlider("feedback", &fHslider1, FAUSTFLOAT(0.9f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->declare(0, "1", "");
		ui_interface->openHorizontalBox("Mod");
		ui_interface->declare(&fHslider3, "06", "");
		ui_interface->declare(&fHslider3, "style", "knob");
		ui_interface->addHorizontalSlider("modDepth", &fHslider3, FAUSTFLOAT(0.1f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1.0f), FAUSTFLOAT(0.001f));
		ui_interface->declare(&fHslider2, "07", "");
		ui_interface->declare(&fHslider2, "style", "knob");
		ui_interface->addHorizontalSlider("modFreq", &fHslider2, FAUSTFLOAT(2.0f), FAUSTFLOAT(0.0f), FAUSTFLOAT(1e+01f), FAUSTFLOAT(0.01f));
		ui_interface->closeBox();
		ui_interface->closeBox();
	}
	
	void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		float fSlow0 = std::max<float>(0.0f, std::min<float>(0.99f, float(fHslider0)));
		float fSlow1 = std::max<float>(0.0f, std::min<float>(1.0f, float(fHslider1)));
		float fSlow2 = std::max<float>(0.0f, std::min<float>(1e+01f, float(fHslider2)));
		float fSlow3 = std::max<float>(0.0f, std::min<float>(1.0f, float(fHslider3)));
		float fSlow4 = std::floor(std::min<float>(65533.0f, fConst0 * std::max<float>(0.001f, std::min<float>(1.45f, float(fHslider4)))));
		float fSlow5 = std::max<float>(0.0f, std::min<float>(0.99f, float(fHslider5)));
		float fSlow6 = std::max<float>(0.5f, std::min<float>(3.0f, float(fHslider6)));
		int iSlow7 = itbl0_reverb_faustSIG0[int(49.0f * fSlow6)];
		float fSlow8 = 0.0001f * float(iSlow7);
		int iSlow9 = itbl0_reverb_faustSIG0[int(59.0f * fSlow6)];
		float fSlow10 = 0.0001f * float(iSlow9);
		int iSlow11 = itbl0_reverb_faustSIG0[int(36.0f * fSlow6)];
		float fSlow12 = 0.0001f * float(iSlow11);
		int iSlow13 = itbl0_reverb_faustSIG0[int(46.0f * fSlow6)];
		float fSlow14 = 0.0001f * float(iSlow13);
		int iSlow15 = itbl0_reverb_faustSIG0[int(23.0f * fSlow6)];
		float fSlow16 = 0.0001f * float(iSlow15);
		int iSlow17 = itbl0_reverb_faustSIG0[int(33.0f * fSlow6)];
		float fSlow18 = 0.0001f * float(iSlow17);
		int iSlow19 = itbl0_reverb_faustSIG0[int(1e+01f * fSlow6)];
		float fSlow20 = 0.0001f * float(iSlow19);
		int iSlow21 = itbl0_reverb_faustSIG0[int(2e+01f * fSlow6)];
		float fSlow22 = 0.0001f * float(iSlow21);
		int iSlow23 = itbl0_reverb_faustSIG0[int(68.0f * fSlow6)];
		float fSlow24 = 0.0001f * float(iSlow23);
		int iSlow25 = itbl0_reverb_faustSIG0[int(78.0f * fSlow6)];
		float fSlow26 = 0.0001f * float(iSlow25);
		int iSlow27 = itbl0_reverb_faustSIG0[int(55.0f * fSlow6)];
		float fSlow28 = 0.0001f * float(iSlow27);
		int iSlow29 = itbl0_reverb_faustSIG0[int(65.0f * fSlow6)];
		float fSlow30 = 0.0001f * float(iSlow29);
		int iSlow31 = itbl0_reverb_faustSIG0[int(42.0f * fSlow6)];
		float fSlow32 = 0.0001f * float(iSlow31);
		int iSlow33 = itbl0_reverb_faustSIG0[int(52.0f * fSlow6)];
		float fSlow34 = 0.0001f * float(iSlow33);
		int iSlow35 = itbl0_reverb_faustSIG0[int(29.0f * fSlow6)];
		float fSlow36 = 0.0001f * float(iSlow35);
		int iSlow37 = itbl0_reverb_faustSIG0[int(39.0f * fSlow6)];
		float fSlow38 = 0.0001f * float(iSlow37);
		int iSlow39 = itbl0_reverb_faustSIG0[int(87.0f * fSlow6)];
		float fSlow40 = 0.0001f * float(iSlow39);
		int iSlow41 = itbl0_reverb_faustSIG0[int(97.0f * fSlow6)];
		float fSlow42 = 0.0001f * float(iSlow41);
		int iSlow43 = itbl0_reverb_faustSIG0[int(74.0f * fSlow6)];
		float fSlow44 = 0.0001f * float(iSlow43);
		int iSlow45 = itbl0_reverb_faustSIG0[int(84.0f * fSlow6)];
		float fSlow46 = 0.0001f * float(iSlow45);
		int iSlow47 = itbl0_reverb_faustSIG0[int(61.0f * fSlow6)];
		float fSlow48 = 0.0001f * float(iSlow47);
		int iSlow49 = itbl0_reverb_faustSIG0[int(71.0f * fSlow6)];
		float fSlow50 = 0.0001f * float(iSlow49);
		int iSlow51 = itbl0_reverb_faustSIG0[int(48.0f * fSlow6)];
		float fSlow52 = 0.0001f * float(iSlow51);
		int iSlow53 = itbl0_reverb_faustSIG0[int(58.0f * fSlow6)];
		float fSlow54 = 0.0001f * float(iSlow53);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec0[0] = 1;
			fVec1[0] = fSlow0;
			float fTemp0 = fSlow0 + fVec1[1];
			fVec2[0] = fSlow1;
			float fTemp1 = fSlow1 + fVec2[1];
			fVec3[0] = fSlow2;
			float fTemp2 = fConst1 * (fSlow2 + fVec3[1]);
			float fTemp3 = std::cos(fTemp2);
			float fTemp4 = std::sin(fTemp2);
			fRec3[0] = fRec4[1] * fTemp4 + fRec3[1] * fTemp3;
			int iTemp5 = 1 - iVec0[1];
			fRec4[0] = float(iTemp5) + fRec4[1] * fTemp3 - fTemp4 * fRec3[1];
			fVec4[0] = fSlow3;
			float fTemp6 = fSlow3 + fVec4[1];
			float fTemp7 = fConst2 * fTemp6 * (fRec4[0] + 1.0f);
			float fTemp8 = fTemp7 + 8.500005f;
			int iTemp9 = int(fTemp8);
			float fTemp10 = std::floor(fTemp8);
			float fTemp11 = fTemp7 + (7.0f - fTemp10);
			float fTemp12 = fTemp7 + (8.0f - fTemp10);
			float fTemp13 = fTemp7 + (9.0f - fTemp10);
			float fTemp14 = fTemp7 + (1e+01f - fTemp10);
			float fTemp15 = fTemp14 * fTemp13;
			float fTemp16 = fTemp15 * fTemp12;
			float fTemp17 = (fTemp7 + (6.0f - fTemp10)) * (fTemp11 * (fTemp12 * (0.041666668f * fRec0[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp9)) + 1)) & 1023] * fTemp13 - 0.16666667f * fTemp14 * fRec0[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp9 + 1)) + 1)) & 1023]) + 0.25f * fTemp15 * fRec0[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp9 + 2)) + 1)) & 1023]) - 0.16666667f * fTemp16 * fRec0[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp9 + 3)) + 1)) & 1023]) + 0.041666668f * fTemp16 * fTemp11 * fRec0[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp9 + 4)) + 1)) & 1023];
			fVec5[IOTA0 & 131071] = fTemp17;
			float fTemp18 = ((fRec5[1] != 0.0f) ? (((fRec6[1] > 0.0f) & (fRec6[1] < 1.0f)) ? fRec5[1] : 0.0f) : (((fRec6[1] == 0.0f) & (fSlow4 != fRec7[1])) ? 4.5351473e-05f : (((fRec6[1] == 1.0f) & (fSlow4 != fRec8[1])) ? -4.5351473e-05f : 0.0f)));
			fRec5[0] = fTemp18;
			fRec6[0] = std::max<float>(0.0f, std::min<float>(1.0f, fRec6[1] + fTemp18));
			fRec7[0] = (((fRec6[1] >= 1.0f) & (fRec8[1] != fSlow4)) ? fSlow4 : fRec7[1]);
			fRec8[0] = (((fRec6[1] <= 0.0f) & (fRec7[1] != fSlow4)) ? fSlow4 : fRec8[1]);
			int iTemp19 = int(std::min<float>(65536.0f, std::max<float>(0.0f, fRec7[0])));
			float fTemp20 = fVec5[(IOTA0 - iTemp19) & 131071];
			int iTemp21 = int(std::min<float>(65536.0f, std::max<float>(0.0f, fRec8[0])));
			float fTemp22 = float(input0[i0]) + 0.5f * (fTemp20 + fRec6[0] * (fVec5[(IOTA0 - iTemp21) & 131071] - fTemp20)) * fTemp1;
			fVec6[0] = fSlow5;
			float fTemp23 = fSlow5 + fVec6[1];
			float fTemp24 = 0.5f * fTemp23;
			float fTemp25 = std::sin(fTemp24);
			float fTemp26 = std::cos(fTemp24);
			float fTemp27 = fConst2 * fTemp6 * (fRec3[0] + 1.0f);
			float fTemp28 = fTemp27 + 8.500005f;
			int iTemp29 = int(fTemp28);
			float fTemp30 = std::floor(fTemp28);
			float fTemp31 = fTemp27 + (7.0f - fTemp30);
			float fTemp32 = fTemp27 + (8.0f - fTemp30);
			float fTemp33 = fTemp27 + (9.0f - fTemp30);
			float fTemp34 = fTemp27 + (1e+01f - fTemp30);
			float fTemp35 = fTemp34 * fTemp33;
			float fTemp36 = fTemp35 * fTemp32;
			float fTemp37 = (fTemp27 + (6.0f - fTemp30)) * (fTemp31 * (fTemp32 * (0.041666668f * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp29)) + 1)) & 1023] * fTemp33 - 0.16666667f * fTemp34 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp29 + 1)) + 1)) & 1023]) + 0.25f * fTemp35 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp29 + 2)) + 1)) & 1023]) - 0.16666667f * fTemp36 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp29 + 3)) + 1)) & 1023]) + 0.041666668f * fTemp36 * fTemp31 * fRec1[(IOTA0 - (std::min<int>(512, std::max<int>(0, iTemp29 + 4)) + 1)) & 1023];
			fVec7[IOTA0 & 131071] = fTemp37;
			float fTemp38 = fVec7[(IOTA0 - iTemp19) & 131071];
			float fTemp39 = float(input1[i0]) + 0.5f * fTemp1 * (fTemp38 + fRec6[0] * (fVec7[(IOTA0 - iTemp21) & 131071] - fTemp38));
			float fTemp40 = fTemp26 * fTemp39 - fTemp25 * fRec10[1];
			float fTemp41 = fTemp26 * fTemp40 - fTemp25 * fRec13[1];
			float fTemp42 = fTemp26 * fTemp41 - fTemp25 * fRec16[1];
			fRec21[0] = 0.9999f * (fRec21[1] + float(iTemp5 * iSlow7)) + fSlow8;
			float fTemp43 = fRec21[0] + -1.49999f;
			float fTemp44 = std::floor(fTemp43);
			fVec8[IOTA0 & 16383] = fTemp25 * fRec19[1] - fTemp26 * fTemp42;
			float fTemp45 = fVec8[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp43)))) & 16383];
			fVec9[0] = fTemp45;
			fRec20[0] = fVec9[1] - (fTemp44 + (2.0f - fRec21[0])) * (fRec20[1] - fTemp45) / (fRec21[0] - fTemp44);
			fRec18[0] = fRec20[0];
			fRec23[0] = 0.9999f * (fRec23[1] + float(iTemp5 * iSlow9)) + fSlow10;
			float fTemp46 = fRec23[0] + -1.49999f;
			float fTemp47 = std::floor(fTemp46);
			float fTemp48 = fTemp22 * fTemp26 - fTemp25 * fRec9[1];
			float fTemp49 = fTemp26 * fTemp48 - fTemp25 * fRec12[1];
			float fTemp50 = fTemp26 * fTemp49 - fTemp25 * fRec15[1];
			fVec10[IOTA0 & 16383] = fTemp26 * fTemp50 - fTemp25 * fRec18[1];
			float fTemp51 = fVec10[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp46)))) & 16383];
			fVec11[0] = fTemp51;
			fRec22[0] = fVec11[1] - (fTemp47 + (2.0f - fRec23[0])) * (fRec22[1] - fTemp51) / (fRec23[0] - fTemp47);
			fRec19[0] = fRec22[0];
			fVec12[IOTA0 & 16383] = fTemp26 * fRec19[1] + fTemp25 * fTemp42;
			fRec24[0] = 0.9999f * (fRec24[1] + float(iTemp5 * iSlow11)) + fSlow12;
			float fTemp52 = fRec24[0] + -1.49999f;
			float fTemp53 = fVec12[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp52)))) & 16383];
			fVec13[0] = fTemp53;
			float fTemp54 = std::floor(fTemp52);
			float fTemp55 = fRec24[0] - fTemp54;
			float fTemp56 = fTemp54 + (2.0f - fRec24[0]);
			fRec17[0] = -(fRec17[1] * fTemp56 / fTemp55 + fTemp56 * fTemp53 / fTemp55 + fVec13[1]);
			fRec15[0] = fRec17[0];
			fRec26[0] = 0.9999f * (fRec26[1] + float(iTemp5 * iSlow13)) + fSlow14;
			float fTemp57 = fRec26[0] + -1.49999f;
			float fTemp58 = std::floor(fTemp57);
			fVec14[IOTA0 & 16383] = fRec18[1] * fTemp26 + fTemp25 * fTemp50;
			float fTemp59 = fVec14[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp57)))) & 16383];
			fVec15[0] = fTemp59;
			fRec25[0] = fVec15[1] - (fTemp58 + (2.0f - fRec26[0])) * (fRec25[1] - fTemp59) / (fRec26[0] - fTemp58);
			fRec16[0] = fRec25[0];
			fVec16[IOTA0 & 16383] = fTemp26 * fRec16[1] + fTemp25 * fTemp41;
			fRec27[0] = 0.9999f * (fRec27[1] + float(iTemp5 * iSlow15)) + fSlow16;
			float fTemp60 = fRec27[0] + -1.49999f;
			float fTemp61 = fVec16[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp60)))) & 16383];
			fVec17[0] = fTemp61;
			float fTemp62 = std::floor(fTemp60);
			float fTemp63 = fRec27[0] - fTemp62;
			float fTemp64 = fTemp62 + (2.0f - fRec27[0]);
			fRec14[0] = -(fRec14[1] * fTemp64 / fTemp63 + fTemp64 * fTemp61 / fTemp63 + fVec17[1]);
			fRec12[0] = fRec14[0];
			fRec29[0] = 0.9999f * (fRec29[1] + float(iTemp5 * iSlow17)) + fSlow18;
			float fTemp65 = fRec29[0] + -1.49999f;
			float fTemp66 = std::floor(fTemp65);
			fVec18[IOTA0 & 16383] = fRec15[1] * fTemp26 + fTemp25 * fTemp49;
			float fTemp67 = fVec18[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp65)))) & 16383];
			fVec19[0] = fTemp67;
			fRec28[0] = fVec19[1] - (fTemp66 + (2.0f - fRec29[0])) * (fRec28[1] - fTemp67) / (fRec29[0] - fTemp66);
			fRec13[0] = fRec28[0];
			fVec20[IOTA0 & 16383] = fTemp26 * fRec13[1] + fTemp25 * fTemp40;
			fRec30[0] = 0.9999f * (fRec30[1] + float(iTemp5 * iSlow19)) + fSlow20;
			float fTemp68 = fRec30[0] + -1.49999f;
			float fTemp69 = fVec20[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp68)))) & 16383];
			fVec21[0] = fTemp69;
			float fTemp70 = std::floor(fTemp68);
			float fTemp71 = fRec30[0] - fTemp70;
			float fTemp72 = fTemp70 + (2.0f - fRec30[0]);
			fRec11[0] = -(fRec11[1] * fTemp72 / fTemp71 + fTemp72 * fTemp69 / fTemp71 + fVec21[1]);
			fRec9[0] = fRec11[0];
			fRec32[0] = 0.9999f * (fRec32[1] + float(iTemp5 * iSlow21)) + fSlow22;
			float fTemp73 = fRec32[0] + -1.49999f;
			float fTemp74 = std::floor(fTemp73);
			fVec22[IOTA0 & 16383] = fRec12[1] * fTemp26 + fTemp25 * fTemp48;
			float fTemp75 = fVec22[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp73)))) & 16383];
			fVec23[0] = fTemp75;
			fRec31[0] = fVec23[1] - (fTemp74 + (2.0f - fRec32[0])) * (fRec31[1] - fTemp75) / (fRec32[0] - fTemp74);
			fRec10[0] = fRec31[0];
			float fTemp76 = fRec9[1] * fTemp26 + fTemp25 * fTemp22;
			float fTemp77 = -0.5f * fTemp23;
			float fTemp78 = std::sin(fTemp77);
			float fTemp79 = std::cos(fTemp77);
			float fTemp80 = fTemp26 * fRec10[1] + fTemp25 * fTemp39;
			float fTemp81 = fTemp79 * fTemp80 - fTemp78 * fRec34[1];
			float fTemp82 = fTemp79 * fTemp81 - fTemp78 * fRec37[1];
			float fTemp83 = fTemp79 * fTemp82 - fTemp78 * fRec40[1];
			fRec45[0] = 0.9999f * (fRec45[1] + float(iTemp5 * iSlow23)) + fSlow24;
			float fTemp84 = fRec45[0] + -1.49999f;
			float fTemp85 = std::floor(fTemp84);
			fVec24[IOTA0 & 16383] = fTemp78 * fRec43[1] - fTemp79 * fTemp83;
			float fTemp86 = fVec24[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp84)))) & 16383];
			fVec25[0] = fTemp86;
			fRec44[0] = fVec25[1] - (fTemp85 + (2.0f - fRec45[0])) * (fRec44[1] - fTemp86) / (fRec45[0] - fTemp85);
			fRec42[0] = fRec44[0];
			fRec47[0] = 0.9999f * (fRec47[1] + float(iTemp5 * iSlow25)) + fSlow26;
			float fTemp87 = fRec47[0] + -1.49999f;
			float fTemp88 = std::floor(fTemp87);
			float fTemp89 = fTemp76 * fTemp79 - fTemp78 * fRec33[1];
			float fTemp90 = fTemp79 * fTemp89 - fTemp78 * fRec36[1];
			float fTemp91 = fTemp79 * fTemp90 - fTemp78 * fRec39[1];
			fVec26[IOTA0 & 16383] = fTemp79 * fTemp91 - fRec42[1] * fTemp78;
			float fTemp92 = fVec26[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp87)))) & 16383];
			fVec27[0] = fTemp92;
			fRec46[0] = fVec27[1] - (fTemp88 + (2.0f - fRec47[0])) * (fRec46[1] - fTemp92) / (fRec47[0] - fTemp88);
			fRec43[0] = fRec46[0];
			fVec28[IOTA0 & 16383] = fTemp79 * fRec43[1] + fTemp78 * fTemp83;
			fRec48[0] = 0.9999f * (fRec48[1] + float(iTemp5 * iSlow27)) + fSlow28;
			float fTemp93 = fRec48[0] + -1.49999f;
			float fTemp94 = fVec28[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp93)))) & 16383];
			fVec29[0] = fTemp94;
			float fTemp95 = std::floor(fTemp93);
			float fTemp96 = fRec48[0] - fTemp95;
			float fTemp97 = fTemp95 + (2.0f - fRec48[0]);
			fRec41[0] = -(fRec41[1] * fTemp97 / fTemp96 + fTemp97 * fTemp94 / fTemp96 + fVec29[1]);
			fRec39[0] = fRec41[0];
			fRec50[0] = 0.9999f * (fRec50[1] + float(iTemp5 * iSlow29)) + fSlow30;
			float fTemp98 = fRec50[0] + -1.49999f;
			float fTemp99 = std::floor(fTemp98);
			fVec30[IOTA0 & 16383] = fRec42[1] * fTemp79 + fTemp78 * fTemp91;
			float fTemp100 = fVec30[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp98)))) & 16383];
			fVec31[0] = fTemp100;
			fRec49[0] = fVec31[1] - (fTemp99 + (2.0f - fRec50[0])) * (fRec49[1] - fTemp100) / (fRec50[0] - fTemp99);
			fRec40[0] = fRec49[0];
			fVec32[IOTA0 & 16383] = fTemp79 * fRec40[1] + fTemp78 * fTemp82;
			fRec51[0] = 0.9999f * (fRec51[1] + float(iTemp5 * iSlow31)) + fSlow32;
			float fTemp101 = fRec51[0] + -1.49999f;
			float fTemp102 = fVec32[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp101)))) & 16383];
			fVec33[0] = fTemp102;
			float fTemp103 = std::floor(fTemp101);
			float fTemp104 = fRec51[0] - fTemp103;
			float fTemp105 = fTemp103 + (2.0f - fRec51[0]);
			fRec38[0] = -(fRec38[1] * fTemp105 / fTemp104 + fTemp105 * fTemp102 / fTemp104 + fVec33[1]);
			fRec36[0] = fRec38[0];
			fRec53[0] = 0.9999f * (fRec53[1] + float(iTemp5 * iSlow33)) + fSlow34;
			float fTemp106 = fRec53[0] + -1.49999f;
			float fTemp107 = std::floor(fTemp106);
			fVec34[IOTA0 & 16383] = fRec39[1] * fTemp79 + fTemp78 * fTemp90;
			float fTemp108 = fVec34[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp106)))) & 16383];
			fVec35[0] = fTemp108;
			fRec52[0] = fVec35[1] - (fTemp107 + (2.0f - fRec53[0])) * (fRec52[1] - fTemp108) / (fRec53[0] - fTemp107);
			fRec37[0] = fRec52[0];
			fVec36[IOTA0 & 16383] = fTemp79 * fRec37[1] + fTemp78 * fTemp81;
			fRec54[0] = 0.9999f * (fRec54[1] + float(iTemp5 * iSlow35)) + fSlow36;
			float fTemp109 = fRec54[0] + -1.49999f;
			float fTemp110 = fVec36[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp109)))) & 16383];
			fVec37[0] = fTemp110;
			float fTemp111 = std::floor(fTemp109);
			float fTemp112 = fRec54[0] - fTemp111;
			float fTemp113 = fTemp111 + (2.0f - fRec54[0]);
			fRec35[0] = -(fRec35[1] * fTemp113 / fTemp112 + fTemp113 * fTemp110 / fTemp112 + fVec37[1]);
			fRec33[0] = fRec35[0];
			fRec56[0] = 0.9999f * (fRec56[1] + float(iTemp5 * iSlow37)) + fSlow38;
			float fTemp114 = fRec56[0] + -1.49999f;
			float fTemp115 = std::floor(fTemp114);
			fVec38[IOTA0 & 16383] = fRec36[1] * fTemp79 + fTemp78 * fTemp89;
			float fTemp116 = fVec38[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp114)))) & 16383];
			fVec39[0] = fTemp116;
			fRec55[0] = fVec39[1] - (fTemp115 + (2.0f - fRec56[0])) * (fRec55[1] - fTemp116) / (fRec56[0] - fTemp115);
			fRec34[0] = fRec55[0];
			float fTemp117 = fRec33[1] * fTemp79 + fTemp78 * fTemp76;
			float fTemp118 = fTemp79 * fRec34[1] + fTemp78 * fTemp80;
			float fTemp119 = fTemp26 * fTemp118 - fTemp25 * fRec58[1];
			float fTemp120 = fTemp26 * fTemp119 - fTemp25 * fRec61[1];
			float fTemp121 = fTemp26 * fTemp120 - fTemp25 * fRec64[1];
			fRec69[0] = 0.9999f * (fRec69[1] + float(iTemp5 * iSlow39)) + fSlow40;
			float fTemp122 = fRec69[0] + -1.49999f;
			float fTemp123 = std::floor(fTemp122);
			fVec40[IOTA0 & 16383] = fTemp25 * fRec67[1] - fTemp26 * fTemp121;
			float fTemp124 = fVec40[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp122)))) & 16383];
			fVec41[0] = fTemp124;
			fRec68[0] = fVec41[1] - (fTemp123 + (2.0f - fRec69[0])) * (fRec68[1] - fTemp124) / (fRec69[0] - fTemp123);
			fRec66[0] = fRec68[0];
			fRec71[0] = 0.9999f * (fRec71[1] + float(iTemp5 * iSlow41)) + fSlow42;
			float fTemp125 = fRec71[0] + -1.49999f;
			float fTemp126 = std::floor(fTemp125);
			float fTemp127 = fTemp26 * fTemp117 - fTemp25 * fRec57[1];
			float fTemp128 = fTemp26 * fTemp127 - fTemp25 * fRec60[1];
			float fTemp129 = fTemp26 * fTemp128 - fTemp25 * fRec63[1];
			fVec42[IOTA0 & 16383] = fTemp26 * fTemp129 - fRec66[1] * fTemp25;
			float fTemp130 = fVec42[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp125)))) & 16383];
			fVec43[0] = fTemp130;
			fRec70[0] = fVec43[1] - (fTemp126 + (2.0f - fRec71[0])) * (fRec70[1] - fTemp130) / (fRec71[0] - fTemp126);
			fRec67[0] = fRec70[0];
			fVec44[IOTA0 & 16383] = fTemp26 * fRec67[1] + fTemp25 * fTemp121;
			fRec72[0] = 0.9999f * (fRec72[1] + float(iTemp5 * iSlow43)) + fSlow44;
			float fTemp131 = fRec72[0] + -1.49999f;
			float fTemp132 = fVec44[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp131)))) & 16383];
			fVec45[0] = fTemp132;
			float fTemp133 = std::floor(fTemp131);
			float fTemp134 = fRec72[0] - fTemp133;
			float fTemp135 = fTemp133 + (2.0f - fRec72[0]);
			fRec65[0] = -(fRec65[1] * fTemp135 / fTemp134 + fTemp135 * fTemp132 / fTemp134 + fVec45[1]);
			fRec63[0] = fRec65[0];
			fRec74[0] = 0.9999f * (fRec74[1] + float(iTemp5 * iSlow45)) + fSlow46;
			float fTemp136 = fRec74[0] + -1.49999f;
			float fTemp137 = std::floor(fTemp136);
			fVec46[IOTA0 & 16383] = fRec66[1] * fTemp26 + fTemp25 * fTemp129;
			float fTemp138 = fVec46[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp136)))) & 16383];
			fVec47[0] = fTemp138;
			fRec73[0] = fVec47[1] - (fTemp137 + (2.0f - fRec74[0])) * (fRec73[1] - fTemp138) / (fRec74[0] - fTemp137);
			fRec64[0] = fRec73[0];
			fVec48[IOTA0 & 16383] = fTemp26 * fRec64[1] + fTemp25 * fTemp120;
			fRec75[0] = 0.9999f * (fRec75[1] + float(iTemp5 * iSlow47)) + fSlow48;
			float fTemp139 = fRec75[0] + -1.49999f;
			float fTemp140 = fVec48[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp139)))) & 16383];
			fVec49[0] = fTemp140;
			float fTemp141 = std::floor(fTemp139);
			float fTemp142 = fRec75[0] - fTemp141;
			float fTemp143 = fTemp141 + (2.0f - fRec75[0]);
			fRec62[0] = -(fRec62[1] * fTemp143 / fTemp142 + fTemp143 * fTemp140 / fTemp142 + fVec49[1]);
			fRec60[0] = fRec62[0];
			fRec77[0] = 0.9999f * (fRec77[1] + float(iTemp5 * iSlow49)) + fSlow50;
			float fTemp144 = fRec77[0] + -1.49999f;
			float fTemp145 = std::floor(fTemp144);
			fVec50[IOTA0 & 16383] = fRec63[1] * fTemp26 + fTemp25 * fTemp128;
			float fTemp146 = fVec50[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp144)))) & 16383];
			fVec51[0] = fTemp146;
			fRec76[0] = fVec51[1] - (fTemp145 + (2.0f - fRec77[0])) * (fRec76[1] - fTemp146) / (fRec77[0] - fTemp145);
			fRec61[0] = fRec76[0];
			fVec52[IOTA0 & 16383] = fTemp26 * fRec61[1] + fTemp25 * fTemp119;
			fRec78[0] = 0.9999f * (fRec78[1] + float(iTemp5 * iSlow51)) + fSlow52;
			float fTemp147 = fRec78[0] + -1.49999f;
			float fTemp148 = fVec52[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp147)))) & 16383];
			fVec53[0] = fTemp148;
			float fTemp149 = std::floor(fTemp147);
			float fTemp150 = fRec78[0] - fTemp149;
			float fTemp151 = fTemp149 + (2.0f - fRec78[0]);
			fRec59[0] = -(fRec59[1] * fTemp151 / fTemp150 + fTemp151 * fTemp148 / fTemp150 + fVec53[1]);
			fRec57[0] = fRec59[0];
			fRec80[0] = 0.9999f * (fRec80[1] + float(iTemp5 * iSlow53)) + fSlow54;
			float fTemp152 = fRec80[0] + -1.49999f;
			float fTemp153 = std::floor(fTemp152);
			fVec54[IOTA0 & 16383] = fRec60[1] * fTemp26 + fTemp25 * fTemp127;
			float fTemp154 = fVec54[(IOTA0 - std::min<int>(8192, std::max<int>(0, int(fTemp152)))) & 16383];
			fVec55[0] = fTemp154;
			fRec79[0] = fVec55[1] - (fTemp153 + (2.0f - fRec80[0])) * (fRec79[1] - fTemp154) / (fRec80[0] - fTemp153);
			fRec58[0] = fRec79[0];
			float fTemp155 = 1.0f - 0.5f * fTemp0;
			fRec2[0] = fTemp155 * (fRec57[1] * fTemp26 + fTemp25 * fTemp117) + 0.5f * fTemp0 * fRec2[1];
			fRec0[IOTA0 & 1023] = fRec2[0];
			fRec81[0] = fTemp155 * (fTemp26 * fRec58[1] + fTemp25 * fTemp118) + 0.5f * fTemp0 * fRec81[1];
			fRec1[IOTA0 & 1023] = fRec81[0];
			output0[i0] = FAUSTFLOAT(fRec0[IOTA0 & 1023]);
			output1[i0] = FAUSTFLOAT(fRec1[IOTA0 & 1023]);
			iVec0[1] = iVec0[0];
			fVec1[1] = fVec1[0];
			fVec2[1] = fVec2[0];
			IOTA0 = IOTA0 + 1;
			fVec3[1] = fVec3[0];
			fRec3[1] = fRec3[0];
			fRec4[1] = fRec4[0];
			fVec4[1] = fVec4[0];
			fRec5[1] = fRec5[0];
			fRec6[1] = fRec6[0];
			fRec7[1] = fRec7[0];
			fRec8[1] = fRec8[0];
			fVec6[1] = fVec6[0];
			fRec21[1] = fRec21[0];
			fVec9[1] = fVec9[0];
			fRec20[1] = fRec20[0];
			fRec18[1] = fRec18[0];
			fRec23[1] = fRec23[0];
			fVec11[1] = fVec11[0];
			fRec22[1] = fRec22[0];
			fRec19[1] = fRec19[0];
			fRec24[1] = fRec24[0];
			fVec13[1] = fVec13[0];
			fRec17[1] = fRec17[0];
			fRec15[1] = fRec15[0];
			fRec26[1] = fRec26[0];
			fVec15[1] = fVec15[0];
			fRec25[1] = fRec25[0];
			fRec16[1] = fRec16[0];
			fRec27[1] = fRec27[0];
			fVec17[1] = fVec17[0];
			fRec14[1] = fRec14[0];
			fRec12[1] = fRec12[0];
			fRec29[1] = fRec29[0];
			fVec19[1] = fVec19[0];
			fRec28[1] = fRec28[0];
			fRec13[1] = fRec13[0];
			fRec30[1] = fRec30[0];
			fVec21[1] = fVec21[0];
			fRec11[1] = fRec11[0];
			fRec9[1] = fRec9[0];
			fRec32[1] = fRec32[0];
			fVec23[1] = fVec23[0];
			fRec31[1] = fRec31[0];
			fRec10[1] = fRec10[0];
			fRec45[1] = fRec45[0];
			fVec25[1] = fVec25[0];
			fRec44[1] = fRec44[0];
			fRec42[1] = fRec42[0];
			fRec47[1] = fRec47[0];
			fVec27[1] = fVec27[0];
			fRec46[1] = fRec46[0];
			fRec43[1] = fRec43[0];
			fRec48[1] = fRec48[0];
			fVec29[1] = fVec29[0];
			fRec41[1] = fRec41[0];
			fRec39[1] = fRec39[0];
			fRec50[1] = fRec50[0];
			fVec31[1] = fVec31[0];
			fRec49[1] = fRec49[0];
			fRec40[1] = fRec40[0];
			fRec51[1] = fRec51[0];
			fVec33[1] = fVec33[0];
			fRec38[1] = fRec38[0];
			fRec36[1] = fRec36[0];
			fRec53[1] = fRec53[0];
			fVec35[1] = fVec35[0];
			fRec52[1] = fRec52[0];
			fRec37[1] = fRec37[0];
			fRec54[1] = fRec54[0];
			fVec37[1] = fVec37[0];
			fRec35[1] = fRec35[0];
			fRec33[1] = fRec33[0];
			fRec56[1] = fRec56[0];
			fVec39[1] = fVec39[0];
			fRec55[1] = fRec55[0];
			fRec34[1] = fRec34[0];
			fRec69[1] = fRec69[0];
			fVec41[1] = fVec41[0];
			fRec68[1] = fRec68[0];
			fRec66[1] = fRec66[0];
			fRec71[1] = fRec71[0];
			fVec43[1] = fVec43[0];
			fRec70[1] = fRec70[0];
			fRec67[1] = fRec67[0];
			fRec72[1] = fRec72[0];
			fVec45[1] = fVec45[0];
			fRec65[1] = fRec65[0];
			fRec63[1] = fRec63[0];
			fRec74[1] = fRec74[0];
			fVec47[1] = fVec47[0];
			fRec73[1] = fRec73[0];
			fRec64[1] = fRec64[0];
			fRec75[1] = fRec75[0];
			fVec49[1] = fVec49[0];
			fRec62[1] = fRec62[0];
			fRec60[1] = fRec60[0];
			fRec77[1] = fRec77[0];
			fVec51[1] = fVec51[0];
			fRec76[1] = fRec76[0];
			fRec61[1] = fRec61[0];
			fRec78[1] = fRec78[0];
			fVec53[1] = fVec53[0];
			fRec59[1] = fRec59[0];
			fRec57[1] = fRec57[0];
			fRec80[1] = fRec80[0];
			fVec55[1] = fVec55[0];
			fRec79[1] = fRec79[0];
			fRec58[1] = fRec58[0];
			fRec2[1] = fRec2[0];
			fRec81[1] = fRec81[0];
		}
	}

};

#ifdef FAUST_UIMACROS
	
	#define FAUST_FILE_NAME "reverb_faust.dsp"
	#define FAUST_CLASS_NAME "_reverb_faust"
	#define FAUST_COMPILATION_OPIONS "-lang cpp -rui -nvi -ct 1 -cn _reverb_faust -scn ::faust::dsp -es 1 -mcd 16 -mdd 1024 -mdy 33 -uim -single -ftz 0"
	#define FAUST_INPUTS 2
	#define FAUST_OUTPUTS 2
	#define FAUST_ACTIVES 7
	#define FAUST_PASSIVES 0

	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[0] Mix/delayTime", fHslider4, 0.2f, 0.001f, 1.45f, 0.0001f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[0] Mix/damping", fHslider0, 0.0f, 0.0f, 0.99f, 0.001f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[0] Mix/size", fHslider6, 1.0f, 0.5f, 3.0f, 0.0001f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[0] Mix/diffusion", fHslider5, 0.5f, 0.0f, 0.99f, 0.0001f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[0] Mix/feedback", fHslider1, 0.9f, 0.0f, 1.0f, 0.01f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[1] Mod/modDepth", fHslider3, 0.1f, 0.0f, 1.0f, 0.001f);
	FAUST_ADDHORIZONTALSLIDER("[0] Greyhole/[1] Mod/modFreq", fHslider2, 2.0f, 0.0f, 1e+01f, 0.01f);

	#define FAUST_LIST_ACTIVES(p) \
		p(HORIZONTALSLIDER, delayTime, "[0] Greyhole/[0] Mix/delayTime", fHslider4, 0.2f, 0.001f, 1.45f, 0.0001f) \
		p(HORIZONTALSLIDER, damping, "[0] Greyhole/[0] Mix/damping", fHslider0, 0.0f, 0.0f, 0.99f, 0.001f) \
		p(HORIZONTALSLIDER, size, "[0] Greyhole/[0] Mix/size", fHslider6, 1.0f, 0.5f, 3.0f, 0.0001f) \
		p(HORIZONTALSLIDER, diffusion, "[0] Greyhole/[0] Mix/diffusion", fHslider5, 0.5f, 0.0f, 0.99f, 0.0001f) \
		p(HORIZONTALSLIDER, feedback, "[0] Greyhole/[0] Mix/feedback", fHslider1, 0.9f, 0.0f, 1.0f, 0.01f) \
		p(HORIZONTALSLIDER, modDepth, "[0] Greyhole/[1] Mod/modDepth", fHslider3, 0.1f, 0.0f, 1.0f, 0.001f) \
		p(HORIZONTALSLIDER, modFreq, "[0] Greyhole/[1] Mod/modFreq", fHslider2, 2.0f, 0.0f, 1e+01f, 0.01f) \

	#define FAUST_LIST_PASSIVES(p) \

#endif

#endif
