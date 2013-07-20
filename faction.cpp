#include "faction.h"
#include "facdata.h"
#include "rng.h"
#include "math.h"
#include "output.h"
#include "omdata.h"
#include "game.h"
#include "catacharset.h"

#include <sstream>

std::string invent_name();
std::string invent_adj();

faction::faction()
{
// debugmsg("Warning: Faction created without UID!");
 name = "";
 values = 0;
 likes_u = 0;
 respects_u = 0;
 known_by_u = false;
 goal = FACGOAL_NULL;
 job1 = FACJOB_NULL;
 job2 = FACJOB_NULL;
 strength = 0;
 sneak = 0;
 crime = 0;
 cult = 0;
 good = 0;
 omx = 0;
 omy = 0;
 mapx = 0;
 mapy = 0;
 size = 0;
 power = 0;
 id = -1;
}

faction::faction(int uid)
{
 name = "";
 values = 0;
 likes_u = 0;
 respects_u = 0;
 known_by_u = false;
 goal = FACGOAL_NULL;
 job1 = FACJOB_NULL;
 job2 = FACJOB_NULL;
 strength = 0;
 sneak = 0;
 crime = 0;
 cult = 0;
 good = 0;
 omx = 0;
 omy = 0;
 mapx = 0;
 mapy = 0;
 size = 0;
 power = 0;
 id = uid;
}


faction::~faction()
{
}

std::string faction::save_info()
{
 std::stringstream dump;
 dump << id << " " << values << " " << goal << " " << job1 << " " << job2 <<
         " " << likes_u << " " << respects_u << " " << known_by_u << " " <<
         strength << " " << sneak << " " << crime << " " << cult << " " <<
         good << " " << omx << " " << omy << " " << mapx << " " << mapy <<
         " " << size << " " << power << " ";
 dump << opinion_of.size() << " ";
 for (int i = 0; i < opinion_of.size(); i++)
  dump << opinion_of[i] << " ";
 dump << name;
 return dump.str();
}

void faction::load_info(std::string data)
{
 std::stringstream dump;
 int valuetmp, goaltmp, jobtmp1, jobtmp2;
 dump << data;
 dump >> id >> valuetmp >> goaltmp >> jobtmp1 >> jobtmp2 >> likes_u >>
         respects_u >> known_by_u >> strength >> sneak >> crime >> cult >>
         good >> omx >> omy >> mapx >> mapy >> size >> power;
 values = valuetmp;
 goal = faction_goal(goaltmp);
 job1 = faction_job(jobtmp1);
 job2 = faction_job(jobtmp2);
 int tmpsize, tmpop;
 dump >> tmpsize;
 for (int i = 0; i < tmpsize; i++) {
  dump >> tmpop;
  opinion_of.push_back(tmpop);
 }
 std::string subname;
 while (dump >> subname)
  name += " " + subname;
}

void faction::randomize()
{
// Set up values
// TODO: Not always in overmap 0,0
 omx = 0;
 omy = 0;
 mapx = rng(OMAPX / 10, OMAPX - OMAPX / 10);
 mapy = rng(OMAPY / 10, OMAPY - OMAPY / 10);
// Pick an overall goal.
 goal = faction_goal(rng(1, NUM_FACGOALS - 1));
 if (one_in(4))
  goal = FACGOAL_NONE;	// Slightly more likely to not have a real goal
 good     = facgoal_data[goal].good;
 strength = facgoal_data[goal].strength;
 sneak    = facgoal_data[goal].sneak;
 crime    = facgoal_data[goal].crime;
 cult     = facgoal_data[goal].cult;
 job1 = faction_job(rng(1, NUM_FACJOBS - 1));
 do
  job2 = faction_job(rng(0, NUM_FACJOBS - 1));
 while (job2 == job1);
 good     += facjob_data[job1].good     + facjob_data[job2].good;
 strength += facjob_data[job1].strength + facjob_data[job2].strength;
 sneak    += facjob_data[job1].sneak    + facjob_data[job2].sneak;
 crime    += facjob_data[job1].crime    + facjob_data[job2].crime;
 cult     += facjob_data[job1].cult     + facjob_data[job2].cult;

 int num_values = 0;
 int tries = 0;
 values = 0;
 do {
  int v = rng(1, NUM_FACVALS - 1);
  if (!has_value(faction_value(v)) && matches_us(faction_value(v))) {
   values |= mfb(v);
   tries = 0;
   num_values++;
   good     += facval_data[v].good;
   strength += facval_data[v].strength;
   sneak    += facval_data[v].sneak;
   crime    += facval_data[v].crime;
   cult     += facval_data[v].cult;
  } else
   tries++;
 } while((one_in(num_values) || one_in(num_values)) && tries < 15);

 std::string noun;
 int sel = 1, best = strength;
 if (sneak > best) {
  sel = 2;
  best = sneak;
 }
 if (crime > best) {
  sel = 3;
  best = crime;
 }
 if (cult > best)
  sel = 4;
 if (strength <= 0 && sneak <= 0 && crime <= 0 && cult <= 0)
  sel = 0;

 switch (sel) {
 case 1:
  noun  = faction_noun_strong[rng(0, 14)];
  power = dice(5, 20);
  size  = dice(5, 6);
  break;
 case 2:
  noun  = faction_noun_sneak [rng(0, 14)];
  power = dice(5, 8);
  size  = dice(5, 8);
  break;
 case 3:
  noun  = faction_noun_crime [rng(0, 14)];
  power = dice(5, 16);
  size  = dice(5, 8);
  break;
 case 4:
  noun  = faction_noun_cult  [rng(0, 14)];
  power = dice(8, 8);
  size  = dice(4, 6);
  break;
 default:
  noun  = faction_noun_none  [rng(0, 14)];
  power = dice(6, 8);
  size  = dice(6, 6);
 }
  
 char buf[128], buf2[256];
 if (one_in(4)) {
  do{
   libintl_sprintf(buf, _("The %1$s of %2$s"), noun.c_str(), invent_name().c_str());
   name = buf;
  }
  while (utf8_width(name.c_str()) > MAX_FAC_NAME_SIZE);
 }
 else if (one_in(2)) {
  do{
   libintl_sprintf(buf, _("The %1$s %2$s"), invent_adj().c_str(), noun.c_str());
   name = buf;
  }
  while (utf8_width(name.c_str()) > MAX_FAC_NAME_SIZE);
 }
 else {
  do {
   std::string adj;
   if (good >= 3)
    adj = faction_adj_pos[rng(0, 14)];
   else if  (good <= -3)
    adj = faction_adj_bad[rng(0, 14)];
   else
    adj = faction_adj_neu[rng(0, 14)];
   libintl_sprintf(buf, _("The %1$s %2$s"), adj.c_str(), noun.c_str());
   if (one_in(4)){
    libintl_sprintf(buf2, _("%1$s of %2$s"), buf, invent_name().c_str());
    name=buf2;
   }else{
    name=buf;
   }
  } while (utf8_width(name.c_str()) > MAX_FAC_NAME_SIZE);
 }
}

void faction::make_army()
{
 name = _("The army");
 omx = 0;
 omy = 0;
 mapx = OMAPX / 2;
 mapy = OMAPY / 2;
 size = OMAPX * 2;
 power = OMAPX;
 goal = FACGOAL_DOMINANCE;
 job1 = FACJOB_MERCENARIES;
 job2 = FACJOB_NULL;
 if (one_in(4))
  values |= mfb(FACVAL_CHARITABLE);
 if (!one_in(4))
  values |= mfb(FACVAL_EXPLORATION);
 if (one_in(3))
  values |= mfb(FACVAL_BIONICS);
 if (one_in(3))
  values |= mfb(FACVAL_ROBOTS);
 if (one_in(4))
  values |= mfb(FACVAL_TREACHERY);
 if (one_in(4))
  values |= mfb(FACVAL_STRAIGHTEDGE);
 if (!one_in(3))
  values |= mfb(FACVAL_LAWFUL);
 if (one_in(8))
  values |= mfb(FACVAL_CRUELTY);
 id = 0;
}

bool faction::has_job(faction_job j)
{
 return (job1 == j || job2 == j);
}

bool faction::has_value(faction_value v)
{
 return values & mfb(v);
}

bool faction::matches_us(faction_value v)
{
 int numvals = 2;
 if (job2 != FACJOB_NULL)
  numvals++;
 for (int i = 0; i < NUM_FACVALS; i++) {
  if (has_value(faction_value(i)))
   numvals++;
 }
 if (has_job(FACJOB_DRUGS) && v == FACVAL_STRAIGHTEDGE)	// Mutually exclusive
  return false;
 int avggood = (good / numvals + good) / 2;
 int avgstrength = (strength / numvals + strength) / 2;
 int avgsneak = (sneak / numvals + sneak / 2);
 int avgcrime = (crime / numvals + crime / 2);
 int avgcult = (cult / numvals + cult / 2);
/*
 debugmsg("AVG: GOO %d STR %d SNK %d CRM %d CLT %d\n\
       VAL: GOO %d STR %d SNK %d CRM %d CLT %d (%s)", avggood, avgstrength,
avgsneak, avgcrime, avgcult, facval_data[v].good, facval_data[v].strength,
facval_data[v].sneak, facval_data[v].crime, facval_data[v].cult,
facval_data[v].name.c_str());
*/
 if ((abs(facval_data[v].good     - avggood)  <= 3 ||
      (avggood >=  5 && facval_data[v].good >=  1) ||
      (avggood <= -5 && facval_data[v].good <=  0))  &&
     (abs(facval_data[v].strength - avgstrength)   <= 5 ||
      (avgstrength >=  5 && facval_data[v].strength >=  3) ||
      (avgstrength <= -5 && facval_data[v].strength <= -1))  &&
     (abs(facval_data[v].sneak    - avgsneak) <= 4 ||
      (avgsneak >=  5 && facval_data[v].sneak >=  1) ||
      (avgsneak <= -5 && facval_data[v].sneak <= -1))  &&
     (abs(facval_data[v].crime    - avgcrime) <= 4 ||
      (avgcrime >=  5 && facval_data[v].crime >=  0) ||
      (avgcrime <= -5 && facval_data[v].crime <= -1))  &&
     (abs(facval_data[v].cult     - avgcult)  <= 3 ||
      (avgcult >=  5 && facval_data[v].cult >=  1) ||
      (avgcult <= -5 && facval_data[v].cult <= -1)))
  return true;
 return false;
}

std::string faction::describe()
{
 std::string ret = name + _(" have the ultimate goal of ") +
                   facgoal_data[goal].name + _(". Their primary concern is ") +
                   facjob_data[job1].name;
 if (job2 == FACJOB_NULL)
  ret += _(".");
 else
  ret += _(", but they are also involved in ") + facjob_data[job2].name + _(".");
 if (values != 0) {
  ret += _(" They are known for ");
  for (int i = 0; i < NUM_FACVALS; i++) {
   if (has_value(faction_value(i)))
    ret += facval_data[i].name + _(", ");
  }
 }
 size_t pos = ret.rfind(_(", "));
 if (pos != std::string::npos) {
  ret.replace(pos, std::string(_(", ")).length(), _("."));
  pos = ret.rfind(_(", "));
  if (pos != std::string::npos)
   ret.replace(pos, std::string(_(", ")).length(), _(", and "));
 }
 return ret;
}

int faction::response_time(game *g)
{
 int base = abs(mapx - g->levx);
 if (abs(mapy - g->levy) > base)
  base = abs(mapy - g->levy);
 if (base > size)	// Out of our sphere of influence
  base *= 2.5;
 base *= 24;	// 24 turns to move one overmap square
 int maxdiv = 10;
 if (goal == FACGOAL_DOMINANCE)
  maxdiv += 2;
 if (has_job(FACJOB_CARAVANS))
  maxdiv += 2;
 if (has_job(FACJOB_SCAVENGE))
  maxdiv++;
 if (has_job(FACJOB_MERCENARIES))
  maxdiv += 2;
 if (has_job(FACJOB_FARMERS))
  maxdiv -= 2;
 if (has_value(FACVAL_EXPLORATION))
  maxdiv += 2;
 if (has_value(FACVAL_LONERS))
  maxdiv -= 3;
 if (has_value(FACVAL_TREACHERY))
  maxdiv -= rng(0, 3);
 int mindiv = (maxdiv > 9 ? maxdiv - 9 : 1);
 base /= rng(mindiv, maxdiv);// We might be in the field
 base -= likes_u;	// We'll hurry, if we like you
 if (base < 100)
  base = 100;
 return base;
}


// END faction:: MEMBER FUNCTIONS


std::string invent_name()
{
 std::string ret = "";
 std::string tmp;
 int syllables = rng(2, 3);
 for (int i = 0; i < syllables; i++) {
  switch (rng(0, 25)) {
   case  0: tmp = _("ab");  break;
   case  1: tmp = _("bon"); break;
   case  2: tmp = _("cor"); break;
   case  3: tmp = _("den"); break;
   case  4: tmp = _("el");  break;
   case  5: tmp = _("fes"); break;
   case  6: tmp = _("gun"); break;
   case  7: tmp = _("hit"); break;
   case  8: tmp = _("id");  break;
   case  9: tmp = _("jan"); break;
   case 10: tmp = _("kal"); break;
   case 11: tmp = _("ler"); break;
   case 12: tmp = _("mal"); break;
   case 13: tmp = _("nor"); break;
   case 14: tmp = _("or");  break;
   case 15: tmp = _("pan"); break;
   case 16: tmp = _("qua"); break;
   case 17: tmp = _("ros"); break;
   case 18: tmp = _("sin"); break;
   case 19: tmp = _("tor"); break;
   case 20: tmp = _("urr"); break;
   case 21: tmp = _("ven"); break;
   case 22: tmp = _("wel"); break;
   case 23: tmp = _("oxo");  break;
   case 24: tmp = _("yen"); break;
   case 25: tmp = _("zu");  break;
  }
  ret += tmp;
 }
 if(std::string(_("lang.do.not.capitalize.first.letter"))!="true")
   ret[0] += 'A' - 'a';
 return ret;
}

std::string invent_adj()
{
 int syllables = dice(2, 2) - 1;
 std::string ret,  tmp;
 switch (rng(0, 25)) {
  case  0: ret = _("Ald");   break;
  case  1: ret = _("Brogg"); break;
  case  2: ret = _("Cald");  break;
  case  3: ret = _("Dredd"); break;
  case  4: ret = _("Eld");   break;
  case  5: ret = _("Forr");  break;
  case  6: ret = _("Gugg");  break;
  case  7: ret = _("Horr");  break;
  case  8: ret = _("Ill");   break;
  case  9: ret = _("Jov");   break;
  case 10: ret = _("Kok");   break;
  case 11: ret = _("Lill");  break;
  case 12: ret = _("Moom");  break;
  case 13: ret = _("Nov");   break;
  case 14: ret = _("Orb");   break;
  case 15: ret = _("Perv");  break;
  case 16: ret = _("Quot");  break;
  case 17: ret = _("Rar");   break;
  case 18: ret = _("Suss");  break;
  case 19: ret = _("Torr");  break;
  case 20: ret = _("Umbr");  break;
  case 21: ret = _("Viv");   break;
  case 22: ret = _("Warr");  break;
  case 23: ret = _("Xen");   break;
  case 24: ret = _("Yend");  break;
  case 25: ret = _("Zor");   break;
 }
 for (int i = 0; i < syllables - 2; i++) {
  switch (rng(0, 17)) {
   case  0: tmp = _("al");   break;
   case  1: tmp = _("arn");  break;
   case  2: tmp = _("astr"); break;
   case  3: tmp = _("antr"); break;
   case  4: tmp = _("ent");  break;
   case  5: tmp = _("ell");  break;
   case  6: tmp = _("ev");   break;
   case  7: tmp = _("emm");  break;
   case  8: tmp = _("empr"); break;
   case  9: tmp = _("ill");  break;
   case 10: tmp = _("ial");  break;
   case 11: tmp = _("ior");  break;
   case 12: tmp = _("ordr"); break;
   case 13: tmp = _("oth");  break;
   case 14: tmp = _("omn");  break;
   case 15: tmp = _("uv");   break;
   case 16: tmp = _("ulv");  break;
   case 17: tmp = _("urn");  break;
  }
  ret += tmp;
 }
 switch (rng(0, 24)) {
  case  0: tmp = "";      break;
  case  1: tmp = _("al");    break;
  case  2: tmp = _("an");    break;
  case  3: tmp = _("ard");   break;
  case  4: tmp = _("ate");   break;
  case  5: tmp = _("e");     break;
  case  6: tmp = _("ed");    break;
  case  7: tmp = _("en");    break;
  case  8: tmp = _("er");    break;
  case  9: tmp = _("ial");   break;
  case 10: tmp = _("ian");   break;
  case 11: tmp = _("iated"); break;
  case 12: tmp = _("ier");   break;
  case 13: tmp = _("ious");  break;
  case 14: tmp = _("ish");   break;
  case 15: tmp = _("ive");   break;
  case 16: tmp = _("oo");    break;
  case 17: tmp = _("or");    break;
  case 18: tmp = _("oth");   break;
  case 19: tmp = _("old");   break;
  case 20: tmp = _("ous");   break;
  case 21: tmp = _("ul");    break;
  case 22: tmp = _("un");    break;
  case 23: tmp = _("ule");   break;
  case 24: tmp = _("y");     break;
 }
 ret += tmp;
 return ret;
}

// Used in game.cpp
std::string fac_ranking_text(int val)
{
 if (val <= -100)
  return _("Archenemy");
 if (val <= -80)
  return _("Wanted Dead");
 if (val <= -60)
  return _("Enemy of the People");
 if (val <= -40)
  return _("Wanted Criminal");
 if (val <= -20)
  return _("Not Welcome");
 if (val <= -10)
  return _("Pariah");
 if (val <=  -5)
  return _("Disliked");
 if (val >= 100)
  return _("Hero");
 if (val >= 80)
  return _("Idol");
 if (val >= 60)
  return _("Beloved");
 if (val >= 40)
  return _("Highly Valued");
 if (val >= 20)
  return _("Valued");
 if (val >= 10)
  return _("Well-Liked");
 if (val >= 5)
  return _("Liked");

 return _("Neutral");
}

// Used in game.cpp
std::string fac_respect_text(int val)
{
// Respected, feared, etc.
 if (val >= 100)
  return _("Legendary");
 if (val >= 80)
  return _("Unchallenged");
 if (val >= 60)
  return _("Mighty");
 if (val >= 40)
  return _("Famous");
 if (val >= 20)
  return _("Well-Known");
 if (val >= 10)
  return _("Spoken Of");

// Disrepected, laughed at, etc.
 if (val <= -100)
  return _("Worthless Scum");
 if (val <= -80)
  return _("Vermin");
 if (val <= -60)
  return _("Despicable");
 if (val <= -40)
  return _("Parasite");
 if (val <= -20)
  return _("Leech");
 if (val <= -10)
  return _("Laughingstock");

 return _("Neutral");
}
