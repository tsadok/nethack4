struct level *
gen_valley(void) {
  INIT_MAZE("valley", ' ');

  NOTELEPORT;
  HARDFLOOR;
  NOMMAP;

  MAP("----------------------------------------------------------------------------"
      "|...S.|..|.....|  |.....-|      |................|   |...............| |...|"
      "|---|.|.--.---.|  |......--- ----..........-----.-----....---........---.-.|"
      "|   |.|.|..| |.| --........| |.............|   |.......---| |-...........--|"
      "|   |...S..| |.| |.......-----.......------|   |--------..---......------- |"
      "|----------- |.| |-......| |....|...-- |...-----................----       |"
      "|.....S....---.| |.......| |....|...|  |..............-----------          |"
      "|.....|.|......| |.....--- |......---  |....---.......|                    |"
      "|.....|.|------| |....--   --....-- |-------- ----....---------------      |"
      "|.....|--......---BBB-|     |...--  |.......|    |..................|      |"
      "|..........||........-|    --...|   |.......|    |...||.............|      |"
      "|.....|...-||-........------....|   |.......---- |...||.............--     |"
      "|.....|--......---...........--------..........| |.......---------...--    |"
      "|.....| |------| |--.......--|   |..B......----- -----....| |.|  |....---  |"
      "|.....| |......--| ------..| |----..B......|       |.--------.-- |-.....---|"
      "|------ |........|  |.|....| |.....----BBBB---------...........---.........|"
      "|       |........|  |...|..| |.....|  |-.............--------...........---|"
      "|       --.....-----------.| |....-----.....----------     |.........----  |"
      "|        |..|..B...........| |.|..........|.|              |.|........|    |"
      "----------------------------------------------------------------------------");

  TEMPLE({1, 6}, {5, 14}, LIT);
  ALTAR({3, 10}, UNALIGNED, SHRINE);

  MORGUE({19, 1}, {24, 8}, LIT, FILLED, true);
  MORGUE({9, 14}, {16, 18}, LIT, FILLED, true);
  MORGUE({37, 9}, {43, 14}, LIT, FILLED, true);

  STAIR_DOWN({1, 1});
  STAIR_UP({66, 17});

  TELEPORT_REGION({58, 9}, {72, 18}, DOWN);
  NON_DIGGABLE({0, 0}, {75, 19});

  OBJECT_MON('%', "corpse", RANDOM_LOC, "archeologist");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "archeologist");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "barbarian");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "barbarian");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "caveman");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "cavewoman");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "healer");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "healer");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "knight");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "knight");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "ranger");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "ranger");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "rogue");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "rogue");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "samurai");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "samurai");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "tourist");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "tourist");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "valkyrie");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "valkyrie");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "wizard");
  OBJECT_MON('%', "corpse", RANDOM_LOC, "wizard");

  OBJECT('*', "ruby", RANDOM_LOC);
  REPEAT(4)
    OBJECT('[', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(4)
    OBJECT(')', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(2)
    OBJECT('*', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(3)
    OBJECT('!', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(3)
    OBJECT('?', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(2)
    OBJECT('/', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(2)
    OBJECT('=', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(2)
    OBJECT('+', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(3)
    OBJECT('(', RANDOM_TYPE, RANDOM_LOC);

  TRAP(SPIKED_PIT, {5, 2});
  TRAP(SPIKED_PIT, {14, 5});
  TRAP(SLEEP_GAS, {14, 5};
  TRAP(BOARD, {21, 12});
  TRAP(BOARD, RANDOM_LOC);
  TRAP(DART, {60, 1});
  TRAP(DART, {26, 17});
  TRAP(ANTI_MAGIC, RANDOM_LOC);
  TRAP(ANTI_MAGIC, RANDOM_LOC);
  TRAP(MAGIC, RANDOM_LOC);
  TRAP(MAGIC, RANDOM_LOC);

  REPEAT(6)
    MON('W', "ghost", RANDOM_LOC);
  REPEAT(3)
    MON('B', "vampire bat", RANDOM_LOC);
  MON('L', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(3)
    MON('V', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(4)
    MON('Z', RANDOM_TYPE, RANDOM_LOC);
  REPEAT(4)
    MON('M', RANDOM_TYPE, RANDOM_LOC);

  LEV_COMP_RETURN;
}
