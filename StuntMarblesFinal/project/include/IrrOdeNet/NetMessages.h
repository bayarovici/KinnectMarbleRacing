enum enumControlCodes {
  eNetAddTorque,
  eNetSetAngularDamping,
  eNetSetPosition,
  eNetSetLinearVelocity,
  eNetSetAngularVelocity
};

enum enumNetMessagCodes {
  eMessageServerFull,           //0
  eMessageClientPing,           //1
  eMessagePlayerDisconnected,   //2
  eNetQueryPlayerName,          //3
  eNetPlayerName,               //4
  eNetSetPlayerName,            //5
  eNetSetTrack,                 //6
  eNetClientTrackSet,           //7
  eNetLoadLevel,                //8
  eNetClientRaceReady,          //9
  eNetClientPlayerId,           //10
  eNetTournamentName,           //11
  eNetAddRace,                  //12
  eNetDelRace,                  //13
  eNetClearList,                //14
  eNetSetRaceTrack,             //15
  eNetSetRaceLaps,              //16
  eNetStartTournament,          //17
  eNetStartRace,                //18
  eNetBackToStart,              //19
  eNetGameRunning               //20
};
