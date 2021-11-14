// file:	Simulator.cpp
//
// summary:	Implements the simulator class. Since this class is memory intensive, it should only be
// included if actual simulation is wished.
#include "Simulator.h"
#include "config.h"
#include "RaceHandler.h"
#include "PROGMEM_readAnything.h"
//#include <avr/pgmspace.h>
/// <summary>
///   Program the interrupt triggers which should be simulated here. See end of file for a collection of records from actual races.
/// </summary>
const SimulatorClass::SimulatorRecord SimulatorClass::SimulatorQueue[60 * NumSimulatedRaces] PROGMEM = {
    // TestCase 0 --> Race3 (GT Team 4):
   {1, -102316, 1},
   {2, -87824, 1},
   {2, -80456, 0},
   {2, -78800, 1},
   {1, 28952, 0},
   {2, 44596, 0},
   {2, 4364748, 1},
   {2, 4366600, 0},
   {2, 4368436, 1},
   {1, 4380404, 1},
   {1, 4382296, 0},
   {1, 4384164, 1},
   {2, 4489344, 0},
   {1, 4505604, 0},
   {1, 4730036, 1},
   {2, 4744732, 1},
   {1, 4857296, 0},
   {2, 4872020, 0},
   {2, 9344132, 1},
   {1, 9361276, 1},
   {1, 9552148, 0},
   {2, 9575988, 0},
   {2, 15477068, 1},
   {1, 15494804, 1},
   {1, 15549608, 0},
   {1, 15563036, 1},
   {2, 15580812, 0},
   {1, 15594584, 0},
   {1, 15610220, 1},
   {1, 15613904, 0},
   {1, 15909920, 1},
   {2, 15925776, 1},
   {1, 16042356, 0},
   {2, 16060036, 0},
   {2, 20674000, 1},
   {1, 20690804, 1},
   {2, 20795000, 0},
   {1, 20803392, 0},
   {1, 21335620, 1},
   {2, 21355152, 1},
   {2, 21469528, 0},
   {2, 21473956, 1},
   {1, 21497288, 0},
   {2, 21521176, 0},
   {2, 26444356, 1},
   {1, 26461276, 1},
   {2, 26582668, 0},
   {1, 26600304, 0},
   {1, 34022032, 1},
   {2, 34130784, 1},
   {2, 35350988, 0},
   {2, 35720860, 1},
   {2, 35731936, 0},
   {2, 35768820, 1},
   {2, 47982712, 0},
   {1, 48095780, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 1: Clean race with positive crosses
   {1, 30249, 1},
   {2, 77339, 1},
   {1, 121961, 0},
   {2, 172825, 0},
   {2, 3773656, 1},
   {1, 3812123, 1},
   {2, 3842430, 0},
   {1, 3882693, 0},
   {1, 4011445, 1},
   {2, 4061786, 1},
   {1, 4104950, 0},
   {2, 4150458, 0},
   {2, 8259300, 1},
   {1, 8296019, 1},
   {2, 8320691, 0},
   {2, 8320751, 1},
   {2, 8322591, 0},
   {1, 8361299, 0},
   {1, 8423019, 1},
   {2, 8468999, 1},
   {1, 8509467, 0},
   {2, 8552556, 0},
   {2, 12787483, 1},
   {1, 12818651, 1},
   {2, 12842055, 0},
   {1, 12873697, 0},
   {1, 12931540, 1},
   {2, 12976724, 1},
   {1, 13017985, 0},
   {2, 13065391, 0},
   {2, 16635371, 1},
   {1, 16683279, 1},
   {2, 16723665, 0},
   {1, 16766664, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 2: Clean race with ok crosses
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4308051, 0},
   {2, 4388118, 1},
   {1, 4451008, 0},
   {2, 4519336, 0},
   {2, 8547315, 1},
   {1, 8597176, 1},
   {2, 8652902, 0},
   {2, 8678511, 1},
   {1, 8723025, 0},
   {2, 8773945, 0},
   {2, 12570143, 1},
   {1, 12614617, 1},
   {2, 12655106, 0},
   {2, 12718378, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 16420845, 1},
   {1, 16478985, 1},
   {2, 16529890, 0},
   {1, 16573259, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 3: Clean race dog 3 with Perfect crossing
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4308051, 0},
   {2, 4388118, 1},
   {1, 4451008, 0},
   {2, 4519336, 0},
   {2, 8547315, 1},
   {1, 8551815, 1},
   {1, 8723025, 0},
   {2, 8773945, 0},
   {2, 12570143, 1},
   {1, 12614617, 1},
   {2, 12655106, 0},
   {2, 12718378, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 16420845, 1},
   {1, 16478985, 1},
   {2, 16529890, 0},
   {1, 16573259, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 4: Clean race dog 4 with Perfect crossing
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4308051, 0},
   {2, 4388118, 1},
   {1, 4451008, 0},
   {2, 4519336, 0},
   {2, 8547315, 1},
   {1, 8597176, 1},
   {2, 8652902, 0},
   {2, 8678511, 1},
   {1, 8723025, 0},
   {2, 8773945, 0},
   {2, 12570143, 1},
   {1, 12573143, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 16420845, 1},
   {1, 16478985, 1},
   {2, 16529890, 0},
   {1, 16573259, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 5: False start + positive crosses
   {1, -708022, 1},
   {2, -668627, 1},
   {1, -635783, 0},
   {2, -595398, 0},
   {2, 4056568, 1},
   {1, 4096181, 1},
   {2, 4127092, 0},
   {1, 4163142, 0},
   {1, 4281161, 1},
   {2, 4335819, 1},
   {1, 4379104, 0},
   {2, 4424379, 0},
   {2, 8734016, 1},
   {1, 8778758, 1},
   {2, 8825067, 0},
   {1, 8868620, 0},
   {1, 8910886, 1},
   {2, 8960478, 1},
   {1, 9007103, 0},
   {2, 9055852, 0},
   {2, 13574890, 1},
   {1, 13619841, 1},
   {2, 13656229, 0},
   {1, 13697372, 0},
   {1, 13776632, 1},
   {2, 13845622, 1},
   {1, 13902723, 0},
   {2, 13959726, 0},
   {2, 18504254, 1},
   {1, 18550742, 1},
   {2, 18600511, 0},
   {1, 18645762, 0},
   {1, 18672299, 1},
   {2, 18734118, 1},
   {1, 18784483, 0},
   {2, 18834598, 0},
   {2, 22926145, 1},
   {1, 22974226, 1},
   {2, 23011816, 0},
   {1, 23057043, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 6: False start + dog 0 re-run fault
   {1, -312563, 1},
   {2, -265863, 1},
   {1, -226969, 0},
   {2, -182394, 0},
   {2, 3435306, 1},
   {1, 3476145, 1},
   {2, 3511277, 0},
   {1, 3550173, 0},
   {1, 3652365, 1},
   {2, 3709525, 1},
   {1, 3757884, 0},
   {2, 3807026, 0},
   {2, 8114079, 1},
   {1, 8161764, 1},
   {2, 8211291, 0},
   {1, 8265749, 0},
   {1, 8271020, 1},
   {2, 8321874, 1},
   {1, 8367949, 0},
   {2, 8417270, 0},
   {2, 12598650, 1},
   {1, 12637656, 1},
   {2, 12670197, 0},
   {1, 12709918, 0},
   {1, 12745147, 1},
   {2, 12803032, 1},
   {1, 12856183, 0},
   {2, 12908647, 0},
   {1, 16158432, 1},
   {2, 16211119, 1},
   {1, 16260650, 0},
   {1, 16269446, 1},
   {2, 16323042, 0},
   {1, 16384003, 0},
   {2, 20939147, 1},
   {1, 20983140, 1},
   {2, 21018309, 0},
   {1, 21055405, 0},
   {1, 21680943, 1},
   {2, 21732824, 1},
   {1, 21776473, 0},
   {2, 21823367, 0},
   {2, 25796547, 1},
   {1, 25841256, 1},
   {2, 25877828, 0},
   {1, 25917047, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 7: False start + dog 0 re-run ok
   {1, -284207, 1},
   {2, -244937, 1},
   {1, -206705, 0},
   {2, -161520, 0},
   {2, 4002259, 1},
   {1, 4044865, 1},
   {2, 4082615, 0},
   {1, 4120866, 0},
   {1, 4245645, 1},
   {2, 4298449, 1},
   {1, 4344289, 0},
   {2, 4397198, 0},
   {2, 9177148, 1},
   {1, 9222798, 1},
   {2, 9273997, 0},
   {1, 9323578, 0},
   {1, 9353128, 1},
   {2, 9406953, 1},
   {1, 9453533, 0},
   {2, 9505698, 0},
   {2, 13706838, 1},
   {1, 13747276, 1},
   {2, 13780047, 0},
   {1, 13817739, 0},
   {1, 13884648, 1},
   {2, 13940043, 1},
   {1, 13988574, 0},
   {2, 14043896, 0},
   {2, 17868801, 1},
   {1, 17921237, 1},
   {1, 18042597, 0},
   {2, 18078320, 0},
   {2, 21974543, 1},
   {1, 22019440, 1},
   {1, 22046641, 0},
   {1, 22047619, 1},
   {2, 22054344, 0},
   {1, 22089903, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 8: False start + dog 4 fault; both rer-runs positive
   {1, -183571, 1},
   {2, -139641, 1},
   {1, -100800, 0},
   {2, -57939, 0},
   {2, 4377258, 1},
   {1, 4419353, 1},
   {2, 4455665, 0},
   {1, 4498609, 0},
   {1, 4611293, 1},
   {2, 4666580, 1},
   {1, 4715197, 0},
   {2, 4765308, 0},
   {2, 9157922, 1},
   {1, 9208897, 1},
   {2, 9266239, 0},
   {1, 9310352, 0},
   {1, 9344484, 1},
   {2, 9401293, 1},
   {1, 9448390, 0},
   {2, 9501722, 0},
   {1, 13385611, 1},
   {2, 13465836, 1},
   {1, 13628626, 0},
   {2, 13642435, 0},
   {2, 17869922, 1},
   {1, 17927898, 1},
   {2, 17979473, 0},
   {1, 18027141, 0},
   {1, 18037726, 1},
   {2, 18089462, 1},
   {1, 18132832, 0},
   {2, 18183083, 0},
   {2, 22689620, 1},
   {1, 22732044, 1},
   {2, 22771055, 0},
   {1, 22813057, 0},
   {1, 22939820, 1},
   {2, 22995954, 1},
   {1, 23045993, 0},
   {2, 23098083, 0},
   {2, 27075292, 1},
   {1, 27121691, 1},
   {2, 27162348, 0},
   {1, 27200596, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 9: Fase start + negative dog 0 first re-run; second re-run positive
   {1, -133962, 1},
   {2, -87647, 1},
   {1, -51148, 0},
   {2, -2471, 0},
   {2, 4191088, 1},
   {1, 4240146, 1},
   {2, 4277973, 0},
   {1, 4321233, 0},
   {1, 4442815, 1},
   {2, 4518124, 1},
   {1, 4582056, 0},
   {2, 4649296, 0},
   {2, 8861527, 1},
   {1, 8912061, 1},
   {2, 8969836, 0},
   {1, 9022882, 0},
   {1, 9109442, 1},
   {2, 9154495, 1},
   {1, 9195812, 0},
   {2, 9246490, 0},
   {2, 13356486, 1},
   {1, 13405954, 1},
   {2, 13446551, 0},
   {1, 13488799, 0},
   {1, 13628005, 1},
   {2, 13702252, 1},
   {1, 13770770, 0},
   {2, 13847047, 0},
   {1, 16874152, 1},
   {2, 16923119, 1},
   {1, 16965808, 0},
   {2, 17011706, 0},
   {2, 17360860, 1},
   {1, 17416942, 1},
   {2, 17470521, 0},
   {1, 17517572, 0},
   {2, 21295366, 1},
   {1, 21353870, 1},
   {2, 21401216, 0},
   {1, 21452574, 0},
   {1, 22458810, 1},
   {2, 22514886, 1},
   {1, 22561531, 0},
   {2, 22613691, 0},
   {2, 27195336, 1},
   {1, 27250351, 1},
   {2, 27294544, 0},
   {1, 27345537, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 10: All dogs faults; re-runs ok
   {1, -53178, 1},
   {2, -12401, 1},
   {1, 22526, 0},
   {2, 65877, 0},
   {1, 4164878, 1},
   {2, 4229409, 1},
   {1, 4286707, 0},
   {1, 4312580, 1},
   {2, 4353636, 0},
   {1, 4404320, 0},
   {1, 8949789, 1},
   {2, 8988462, 1},
   {1, 9020224, 0},
   {1, 9046616, 1},
   {2, 9084291, 0},
   {1, 9127615, 0},
   {1, 13848191, 1},
   {2, 13900633, 1},
   {1, 14039282, 0},
   {2, 14048680, 0},
   {2, 18224099, 1},
   {1, 18266118, 1},
   {1, 18401440, 0},
   {2, 18429996, 0},
   {2, 22717674, 1},
   {1, 22764046, 1},
   {2, 22805511, 0},
   {2, 22858897, 1},
   {1, 22926024, 0},
   {2, 23006942, 0},
   {2, 27306487, 1},
   {1, 27360521, 1},
   {2, 27411993, 0},
   {2, 27430694, 1},
   {1, 27482837, 0},
   {2, 27544708, 0},
   {2, 31483505, 1},
   {1, 31532689, 1},
   {2, 31573942, 0},
   {2, 31663858, 1},
   {1, 31719309, 0},
   {2, 31784676, 0},
   {2, 35539673, 1},
   {1, 35590390, 1},
   {2, 35638008, 0},
   {1, 35677327, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 11: All dogs faults; re-runs positive
   {1, -414511, 1},
   {2, -372584, 1},
   {1, -336998, 0},
   {2, -295959, 0},
   {1, 4304715, 1},
   {2, 4372468, 1},
   {1, 4544306, 0},
   {2, 4590419, 0},
   {1, 8506023, 1},
   {2, 8565499, 1},
   {1, 8618780, 0},
   {1, 8676900, 1},
   {2, 8768382, 0},
   {1, 8845005, 0},
   {1, 13310250, 1},
   {2, 13392579, 1},
   {1, 13563442, 0},
   {2, 13573068, 0},
   {2, 17585938, 1},
   {1, 17643158, 1},
   {2, 17706275, 0},
   {1, 17758585, 0},
   {1, 17944382, 1},
   {2, 17997954, 1},
   {1, 18045061, 0},
   {2, 18098420, 0},
   {2, 22255963, 1},
   {1, 22300629, 1},
   {2, 22339219, 0},
   {1, 22381675, 0},
   {1, 22598326, 1},
   {2, 22666267, 1},
   {1, 22725172, 0},
   {2, 22787163, 0},
   {2, 26808459, 1},
   {1, 26869961, 1},
   {2, 26921211, 0},
   {1, 26969601, 0},
   {1, 27065487, 1},
   {2, 27124398, 1},
   {1, 27174718, 0},
   {2, 27228993, 0},
   {2, 31016993, 1},
   {1, 31069339, 1},
   {2, 31114051, 0},
   {1, 31161253, 0},
   {1, 31315947, 1},
   {2, 31379622, 1},
   {1, 31435745, 0},
   {2, 31495407, 0},
   {2, 34924147, 1},
   {1, 34972741, 1},
   {2, 35022342, 0},
   {1, 35061633, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 12: Dog 2 negative cross
   {1, 155738, 1},
   {2, 199448, 1},
   {1, 236709, 0},
   {2, 282815, 0},
   {1, 3195508, 1},
   {2, 3247969, 1},
   {1, 3294092, 0},
   {2, 3343253, 0},
   {2, 3487840, 1},
   {1, 3554581, 1},
   {2, 3602835, 0},
   {1, 3651407, 0},
   {2, 8031715, 1},
   {1, 8060570, 1},
   {1, 8207830, 0},
   {2, 8222266, 0},
   {2, 12776299, 1},
   {1, 12790085, 1},
   {1, 12916832, 0},
   {2, 12985563, 0},
   {2, 18597673, 1},
   {1, 18658340, 1},
   {2, 18702023, 0},
   {1, 18758446, 0},
   {1, 18836159, 1},
   {2, 18893677, 1},
   {1, 18936505, 0},
   {2, 18983855, 0},
   {2, 23486777, 1},
   {1, 23532142, 1},
   {2, 23570438, 0},
   {1, 23613133, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 13: Dog 2 double fault
   {1, 159967, 1},
   {2, 197366, 1},
   {1, 230368, 0},
   {2, 272216, 0},
   {1, 3102508, 1},
   {2, 3177406, 1},
   {1, 3241558, 0},
   {1, 3274981, 1},
   {2, 3311798, 0},
   {1, 3347154, 0},
   {2, 8317641, 1},
   {1, 8368129, 1},
   {2, 8420492, 0},
   {2, 8467324, 1},
   {1, 8503878, 0},
   {2, 8543883, 0},
   {2, 12658408, 1},
   {1, 12699224, 1},
   {2, 12736667, 0},
   {2, 12784584, 1},
   {1, 12834753, 0},
   {2, 12888053, 0},
   {1, 16581525, 1},
   {2, 16630057, 1},
   {1, 16676575, 0},
   {1, 16687119, 1},
   {2, 16732129, 0},
   {1, 16793009, 0},
   {2, 20707167, 1},
   {1, 20748919, 1},
   {2, 20786805, 0},
   {1, 20824610, 0},
   {1, 21998431, 1},
   {2, 22044086, 1},
   {1, 22082921, 0},
   {2, 22132549, 0},
   {2, 26355853, 1},
   {1, 26403363, 1},
   {2, 26441920, 0},
   {1, 26482582, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 14: Dog 3 negative with re-run ok; dog 4 fault with re-run positive
   {1, 333351, 1},
   {2, 373456, 1},
   {1, 408999, 0},
   {2, 451988, 0},
   {2, 4767154, 1},
   {1, 4805148, 1},
   {2, 4839976, 0},
   {2, 4879371, 1},
   {1, 4933584, 0},
   {2, 4993305, 0},
   {1, 9349056, 1},
   {2, 9386674, 1},
   {1, 9421198, 0},
   {2, 9458100, 0},
   {2, 9704620, 1},
   {1, 9744872, 1},
   {2, 9785937, 0},
   {1, 9823990, 0},
   {1, 14014381, 1},
   {2, 14072511, 1},
   {2, 14201748, 0},
   {1, 14206150, 0},
   {2, 18658029, 1},
   {1, 18713124, 1},
   {2, 18768596, 0},
   {2, 18771907, 1},
   {1, 18825633, 0},
   {2, 18862071, 0},
   {2, 23755239, 1},
   {1, 23798828, 1},
   {2, 23835165, 0},
   {1, 23879765, 0},
   {1, 24002890, 1},
   {2, 24055123, 1},
   {1, 24101418, 0},
   {2, 24151403, 0},
   {2, 27980212, 1},
   {1, 28026004, 1},
   {2, 28071299, 0},
   {1, 28108874, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 15: Dog 3 fault with ok re-run. Dog 4 fautl with re-run negative and second positive
   {1, 230734, 1},
   {2, 273519, 1},
   {1, 308132, 0},
   {2, 350028, 0},
   {2, 5551974, 1},
   {1, 5591980, 1},
   {2, 5626789, 0},
   {2, 5626984, 1},
   {2, 5628822, 0},
   {2, 5655674, 1},
   {1, 5701050, 0},
   {2, 5752590, 0},
   {1, 10019227, 1},
   {2, 10067126, 1},
   {1, 10108949, 0},
   {1, 10195123, 1},
   {2, 10256215, 0},
   {1, 10320886, 0},
   {1, 14574895, 1},
   {2, 14640556, 1},
   {2, 14765903, 0},
   {1, 14777193, 0},
   {2, 19078752, 1},
   {1, 19090103, 1},
   {2, 19243665, 0},
   {1, 19262289, 0},
   {1, 22284328, 1},
   {2, 22342700, 1},
   {1, 22389880, 0},
   {2, 22443015, 0},
   {2, 22538197, 1},
   {1, 22597419, 1},
   {2, 22645083, 0},
   {1, 22695936, 0},
   {2, 26475300, 1},
   {1, 26530359, 1},
   {2, 26584207, 0},
   {1, 26639258, 0},
   {1, 27473138, 1},
   {2, 27517478, 1},
   {1, 27559343, 0},
   {2, 27605788, 0},
   {2, 31299753, 1},
   {1, 31344480, 1},
   {2, 31385491, 0},
   {1, 31424363, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 16 --> False start with late re-run (3 seconds):
   {1, -708022, 1},
   {2, -668627, 1},
   {1, -635783, 0},
   {2, -595398, 0},
   {2, 4056568, 1},
   {1, 4096181, 1},
   {2, 4127092, 0},
   {1, 4163142, 0},
   {1, 4281161, 1},
   {2, 4335819, 1},
   {1, 4379104, 0},
   {2, 4424379, 0},
   {2, 8734016, 1},
   {1, 8778758, 1},
   {2, 8825067, 0},
   {1, 8868620, 0},
   {1, 8910886, 1},
   {2, 8960478, 1},
   {1, 9007103, 0},
   {2, 9055852, 0},
   {2, 13574890, 1},
   {1, 13619841, 1},
   {2, 13656229, 0},
   {1, 13697372, 0},
   {1, 13776632, 1},
   {2, 13845622, 1},
   {1, 13902723, 0},
   {2, 13959726, 0},
   {2, 18504254, 1},
   {1, 18550742, 1},
   {2, 18600511, 0},
   {1, 18645762, 0},
   {1, 21672299, 1},
   {2, 21734118, 1},
   {1, 21784483, 0},
   {2, 21834598, 0},
   {2, 25926145, 1},
   {1, 25974226, 1},
   {2, 26011816, 0},
   {1, 26057043, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 17 --> False start + dog 2 ok:
   {1, -708022, 1},
   {2, -668627, 1},
   {1, -635783, 0},
   {2, -595398, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4308051, 0},
   {2, 4388118, 1},
   {1, 4451008, 0},
   {2, 4519336, 0},
   {2, 8734016, 1},
   {1, 8778758, 1},
   {2, 8825067, 0},
   {1, 8868620, 0},
   {1, 8910886, 1},
   {2, 8960478, 1},
   {1, 9007103, 0},
   {2, 9055852, 0},
   {2, 13574890, 1},
   {1, 13619841, 1},
   {2, 13656229, 0},
   {1, 13697372, 0},
   {1, 13776632, 1},
   {2, 13845622, 1},
   {1, 13902723, 0},
   {2, 13959726, 0},
   {2, 18504254, 1},
   {1, 18550742, 1},
   {2, 18600511, 0},
   {1, 18645762, 0},
   {1, 18672299, 1},
   {2, 18734118, 1},
   {1, 18784483, 0},
   {2, 18834598, 0},
   {2, 22926145, 1},
   {1, 22974226, 1},
   {2, 23011816, 0},
   {1, 23057043, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 18: Clean race with ok crosses, dog 2 enter as invisible dog
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4451008, 0},
   {1, 4519336, 0},
   {2, 8547315, 1},
   {1, 8597176, 1},
   {2, 8652902, 0},
   {2, 8678511, 1},
   {1, 8723025, 0},
   {2, 8773945, 0},
   {2, 12570143, 1},
   {1, 12614617, 1},
   {2, 12655106, 0},
   {2, 12718378, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 16420845, 1},
   {1, 16478985, 1},
   {2, 16529890, 0},
   {1, 16573259, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 19: Dog 2 enter as invisible dog + negative cross of dog 3
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4451008, 0},
   {1, 4519336, 0},
   {1, 9349056, 1},
   {2, 9386674, 1},
   {1, 9421198, 0},
   {2, 9458100, 0},
   {2, 9704620, 1},
   {1, 9744872, 1},
   {2, 9785937, 0},
   {1, 9823990, 0},
   {2, 12570143, 1},
   {1, 12614617, 1},
   {2, 12655106, 0},
   {2, 12718378, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 18597673, 1},
   {1, 18658340, 1},
   {2, 18702023, 0},
   {1, 18758446, 0},
   {1, 18836159, 1},
   {2, 18893677, 1},
   {1, 18936505, 0},
   {2, 18983855, 0},
   {2, 23486777, 1},
   {1, 23532142, 1},
   {2, 23570438, 0},
   {1, 23613133, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 20: Dog 2 missed gate while going in (dog 2 manual fault)
   {1, 49140, 1},
   {2, 90878, 1},
   {1, 128477, 0},
   {2, 172676, 0},
   {2, 4231363, 1},
   {1, 4272941, 1},
   {2, 4451008, 0},
   {1, 4519336, 0},
   {2, 8547315, 1},
   {1, 8597176, 1},
   {2, 8652902, 0},
   {2, 8678511, 1},
   {1, 8723025, 0},
   {2, 8773945, 0},
   {2, 12570143, 1},
   {1, 12614617, 1},
   {2, 12655106, 0},
   {2, 12718378, 1},
   {1, 12780337, 0},
   {2, 12851298, 0},
   {2, 18597673, 1},
   {1, 18658340, 1},
   {2, 18702023, 0},
   {1, 18758446, 0},
   {1, 18836159, 1},
   {2, 18893677, 1},
   {1, 18936505, 0},
   {2, 18983855, 0},
   {2, 23486777, 1},
   {1, 23532142, 1},
   {2, 23570438, 0},
   {1, 23613133, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 21: Training 21.04.2021 lot of sensor noise
   {1, 58718, 1},
   {2, 79076, 1},
   {1, 160867, 0},
   {1, 167573, 1},
   {1, 184489, 0},
   {2, 199788, 0},
   {2, 4686891, 1},
   {1, 4704915, 1},
   {1, 4712182, 0},
   {1, 4713985, 1},
   {1, 4854175, 0},
   {2, 4871471, 0},
   {1, 9438170, 1},
   {2, 9446020, 1},
   {2, 9581382, 0},
   {1, 9585687, 0},
   {1, 9586640, 1},
   {1, 9588435, 0},
   {1, 9590986, 1},
   {1, 9600018, 0},
   {2, 13352143, 1},
   {1, 13365903, 1},
   {2, 13459246, 0},
   {1, 13476056, 0},
   {1, 13731071, 1},
   {2, 13748659, 1},
   {1, 13814050, 0},
   {1, 13824928, 1},
   {1, 13828704, 0},
   {1, 13830698, 1},
   {2, 13835204, 0},
   {1, 13837134, 0},
   {2, 13840537, 1},
   {2, 13850766, 0},
   {2, 13853049, 1},
   {2, 13854764, 0},
   {2, 18117942, 1},
   {1, 18137551, 1},
   {2, 18219072, 0},
   {2, 18232612, 1},
   {2, 18234428, 0},
   {1, 18239133, 0},
   {1, 18246397, 1},
   {1, 18253535, 0},
   {1, 21991786, 1},
   {2, 22011432, 1},
   {1, 22118996, 0},
   {2, 22139606, 0},
   {2, 26139699, 1},
   {1, 26159458, 1},
   {2, 26263700, 0},
   {1, 26281524, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
    // TestCase 22 --> Race3 (GT Team 4) with false start changed to -9ms
   {1, -9316, 1},
   {2, 5176, 1},
   {2, 12544, 0},
   {2, 14200, 1},
   {1, 121952, 0},
   {2, 137596, 0},
   {2, 4364748, 1},
   {2, 4366600, 0},
   {2, 4368436, 1},
   {1, 4380404, 1},
   {1, 4382296, 0},
   {1, 4384164, 1},
   {2, 4489344, 0},
   {1, 4505604, 0},
   {1, 4730036, 1},
   {2, 4744732, 1},
   {1, 4857296, 0},
   {2, 4872020, 0},
   {2, 9344132, 1},
   {1, 9361276, 1},
   {1, 9552148, 0},
   {2, 9575988, 0},
   {2, 15477068, 1},
   {1, 15494804, 1},
   {1, 15549608, 0},
   {1, 15563036, 1},
   {2, 15580812, 0},
   {1, 15594584, 0},
   {1, 15610220, 1},
   {1, 15613904, 0},
   {1, 15909920, 1},
   {2, 15925776, 1},
   {1, 16042356, 0},
   {2, 16060036, 0},
   {2, 20674000, 1},
   {1, 20690804, 1},
   {2, 20795000, 0},
   {1, 20803392, 0},
   {1, 21335620, 1},
   {2, 21355152, 1},
   {2, 21469528, 0},
   {2, 21473956, 1},
   {1, 21497288, 0},
   {2, 21521176, 0},
   {2, 26444356, 1},
   {1, 26461276, 1},
   {2, 26582668, 0},
   {1, 26600304, 0},
   {1, 34022032, 1},
   {2, 34130784, 1},
   {2, 35350988, 0},
   {2, 35720860, 1},
   {2, 35731936, 0},
   {2, 35768820, 1},
   {2, 47982712, 0},
   {1, 48095780, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 23 --> Dog 3 false ok crossing while re-run (BAbBab)
   {1, 152328, 1},
   {2, 173930, 1},
   {1, 299271, 0},
   {2, 313151, 0},
   {2, 4723512, 1},
   {1, 4744508, 1},
   {1, 4940785, 0},
   {2, 4952333, 0},
   {1, 9447536, 1},
   {2, 9447762, 1},
   {2, 9577519, 0},
   {1, 9591404, 0},
   {2, 13228803, 1},
   {1, 13242661, 1},
   {2, 13343681, 0},
   {1, 13360428, 0},
   {1, 13455500, 1},
   {2, 13471068, 1},
   {1, 13535884, 0},
   {1, 13540058, 1},
   {1, 13545958, 0},
   {1, 13548049, 1},
   {1, 13549925, 0},
   {2, 13553607, 0},
   {2, 13555824, 1},
   {2, 13557644, 0},
   {2, 13558739, 1},
   {2, 13562193, 0},
   {2, 13566310, 1},
   {2, 13569729, 0},
   {2, 17710595, 1},
   {1, 17730641, 1},
   {2, 17799022, 0},
   {2, 17815640, 1},
   {1, 17818085, 0},
   {2, 17821105, 0},
   {1, 22457493, 1},
   {2, 22474726, 1},
   {2, 22478358, 0},
   {2, 22480161, 1},
   {1, 22580372, 0},
   {2, 22596483, 0},
   {2, 26379804, 1},
   {1, 26396821, 1},
   {2, 26498859, 0},
   {1, 26518161, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   // TestCase 24 --> test
   {1, 18375, 1},
   {2, 34340, 1},
   {1, 160821, 0},
   {2, 177158, 0},
   {2, 4623881, 1},
   {1, 4641051, 1},
   {1, 4648253, 0},
   {1, 4650037, 1},
   {1, 4841331, 0},
   {1, 4844919, 1},
   {1, 4846735, 0},
   {2, 4863088, 0},
   {1, 9359639, 1},
   {2, 9373075, 1},
   {2, 9516907, 0},
   {1, 9526068, 0},
   {1, 9538526, 1},
   {1, 9542105, 0},
   {2, 13189687, 1},
   {1, 13206629, 1},
   {1, 13377462, 0},
   {1, 13382791, 1},
   {1, 13389935, 0},
   {2, 13398068, 0},
   {2, 13401239, 1},
   {2, 13410721, 0},
   {2, 17714852, 1},
   {1, 17738346, 1},
   {2, 17833095, 0},
   {2, 17842307, 1},
   {2, 17849558, 0},
   {1, 17856188, 0},
   {1, 18898709, 1},
   {2, 18924642, 1},
   {1, 19038168, 0},
   {2, 19059436, 0},
   {1, 20329746, 1},
   {2, 20353373, 1},
   {1, 20444707, 0},
   {2, 20467092, 0},
   {2, 23206430, 1},
   {1, 23222516, 1},
   {2, 23333785, 0},
   {1, 23355576, 0},
   {2, 24568667, 1},
   {1, 24588894, 1},
   {2, 24684249, 0},
   {2, 24698580, 1},
   {2, 24702214, 0},
   {1, 24703228, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0},
   {0, 0, 0}
   //
};
/// <summary>
///   Initialises this object.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void SimulatorClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   _iS1Pin = iS1Pin;
   //pinMode(_iS1Pin, OUTPUT);
   //digitalWrite(_iS1Pin, LOW);
   _iS2Pin = iS2Pin;
   //pinMode(_iS2Pin, OUTPUT);
   //digitalWrite(_iS2Pin, LOW);
   _iDataPos = 0;
   _iDataStartPos = 0;
   PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
   ESP_LOGD(__FILE__, "Simulated Race 0 selected!");
}

/// <summary>
///   Main entry-point for this application. Should be called every main loop cycle if simulation is wished.
/// </summary>
void SimulatorClass::Main()
{
   if (RaceHandler.RaceState == RaceHandler.RESET)
   {
      if (_iDataPos != _iDataStartPos)
      {
         //We've reset a race, reset data position to 0
         _iDataPos = _iDataStartPos;
         PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
      }
      return;
   }
   if (PendingRecord.llTriggerTime == 0)
   {
      //Pending record doesn't contain valid data, this means we've reched the end of our queue
      return;
   }

   //Simulate sensors  
   if ((PendingRecord.llTriggerTime < 0 && RaceHandler.RaceState == RaceHandler.STARTING) || (PendingRecord.llTriggerTime > 0 && RaceHandler.RaceState == RaceHandler.RUNNING)
      || (RaceHandler.RaceState == RaceHandler.STOPPED && GET_MICROS <= RaceHandler._llRaceEndTime + 2000000))
   {
      while (PendingRecord.llTriggerTime != 0 && PendingRecord.llTriggerTime <= (long long)(GET_MICROS - (RaceHandler.llRaceStartTime - 8000))) //8ms advance added
      {
         //ESP_LOGD(__FILE__, "%lld Pending record S%d TriggerTime %lld | %lld", GET_MICROS, PendingRecord.iSensorNumber, RaceHandler.llRaceStartTime + PendingRecord.llTriggerTime, PendingRecord.llTriggerTime);
         RaceHandler._QueuePush({PendingRecord.iSensorNumber, (RaceHandler.llRaceStartTime + PendingRecord.llTriggerTime), PendingRecord.iState});
         //And increase pending record
         _iDataPos++;
         PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
      }
   }
}

void SimulatorClass::ChangeSimulatedRaceID(uint iSimulatedRaceID)
{
   if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
   {
      _iDataStartPos = 60 * iSimulatedRaceID;
      _iDataPos = _iDataStartPos;
      PROGMEM_readAnything(&SimulatorQueue[_iDataPos], PendingRecord);
      ESP_LOGD(__FILE__, "Simulated Race %i selected!", iSimulatedRaceID);
   }
   return;
}
/// <summary>
///   The simulator class object.
/// </summary>
SimulatorClass Simulator;

