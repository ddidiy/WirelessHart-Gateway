
/* DEVICES SECTION */
CREATE TABLE Devices
(
    DeviceID                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,	
    DeviceRole              INTEGER NOT NULL,                               /*  
                                                                             *  1 - NM (Network Manager)
                                                                             *  2 - GW (Gateway)
                                                                             *  4 - AP (Access Point)
                                                                             * 10 - RD (Routing Device)
                                                                             * -1 - Unknown
                                                                             */
    DeviceCode              INTEGER NOT NULL,
    SoftwareRevision        INTEGER NOT NULL,
    Address64               VARCHAR(50) NOT NULL,                           /*	Addr, ex: "001B:1EF9:8100:0002"	*/
    DeviceTag               VARCHAR(64) NOT NULL,                           /*  Tag (Latin1) save as hexstring */
    Nickname                INTEGER NOT NULL,
    DeviceStatus            INTEGER NOT NULL,
    DeviceLevel             INTEGER NOT NULL,                               /*  the level of device in topology */
    RejoinCount             INTEGER,
    LastRead                DATETIME,                                       /*  last time when burst was received from device */
    PowerSupplyStatus       INTEGER NOT NULL,
    PublishStatus           INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE DeviceHistory
(
    HistoryID               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    DeviceID                INTEGER NOT NULL,
    Timestamp               DATETIME NOT NULL,
    DeviceStatus            INTEGER NOT NULL
);

CREATE TABLE DeviceConnections
(
    DeviceID                INTEGER NOT NULL,
    IP                      VARCHAR(50) NOT NULL,
    Port                    INTEGER NOT NULL    
);
CREATE UNIQUE INDEX IX_DeviceConnections ON DeviceConnections(DeviceID);

CREATE TABLE DevicesCodes
(
    DeviceCode              INTEGER NOT NULL PRIMARY KEY,
    Model                   VARCHAR(32) NULL,
    Company                 VARCHAR(64) NULL
);

/* READINGS SECTION */
CREATE TABLE Channels
(
    ChannelID               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    DeviceID                INTEGER NOT NULL,
    BurstMessage            INTEGER NOT NULL,
    DeviceVariableSlot      INTEGER NOT NULL,
    Name                    VARCHAR(16) NOT NULL,
    CmdNo                   INTEGER NOT NULL,                           /*  whart command no. */
    DeviceVariable          INTEGER NOT NULL,                           /*  variable_code (245-primary, 246-secondary,...) */		
    Classification          INTEGER NOT NULL,                           /*  variable_classification_code (64-temperature, 65-pressure,...) */
    UnitCode                INTEGER NOT NULL                            /*  unit_code (32-degreesCelsius, 7-bars,...) -> see common tables */
);

CREATE TABLE ChannelsHistory
(
    ChannelID               INTEGER NOT NULL PRIMARY KEY,
    DeviceID                INTEGER NOT NULL,
    BurstMessage            INTEGER NOT NULL,
    DeviceVariableSlot      INTEGER NOT NULL,
    Name                    VARCHAR(16) NOT NULL,
    CmdNo                   INTEGER NOT NULL,                           /*  whart command no. */
    DeviceVariable          INTEGER NOT NULL,                           /*  variable_code (245-primary, 246-secondary,...) */		
    Classification          INTEGER NOT NULL,                           /*  variable_classification_code (64-temperature, 65-pressure,...) */
    UnitCode                INTEGER NOT NULL,                           /*  unit_code (32-degreesCelsius, 7-bars,...) -> see common tables */
    Timestamp               DATETIME
);

CREATE TABLE Readings
(
    ChannelID               INTEGER NOT NULL,
    ReadingTime             DATETIME NOT NULL,
    Miliseconds             INTEGER NOT NULL DEFAULT '0',
    Value                   DOUBLE  NOT NULL,
    ValueType               INTEGER NOT NULL DEFAULT '0',               /* 0 - normal float, 1 - Infinity, 2 - NaN */
    Status                  INTEGER,
    CommandID               INTEGER NOT NULL,

    PRIMARY KEY (ChannelID)
);

CREATE TABLE BurstMessages
(
    DeviceID                INTEGER NOT NULL,
    BurstMessage            INTEGER NOT NULL,
    CommandNumber           INTEGER NOT NULL,
    UpdatePeriod            DOUBLE  NOT NULL,
    MaxUpdatePeriod         DOUBLE  NOT NULL,
    SubDeviceMAC            VARCHAR(50),

    PRIMARY KEY (DeviceID, BurstMessage, CommandNumber)
);

CREATE TABLE BurstCounters
(
    DeviceID                INTEGER NOT NULL,
    BurstMessage            INTEGER NOT NULL,
    CommandNumber           INTEGER NOT NULL,
    LastUpdate              DATETIME NOT NULL,
    Received                INTEGER NOT NULL,
    Missed                  INTEGER NOT NULL,

    PRIMARY KEY (DeviceID,BurstMessage,CommandNumber)
);

CREATE TABLE BurstTriggers
(
    DeviceID                INTEGER NOT NULL,
    BurstMessage            INTEGER NOT NULL,
    CommandNumber           INTEGER NOT NULL,
    ModeSelection           INTEGER NOT NULL,
    Classification          INTEGER NOT NULL,
    UnitCode                INTEGER NOT NULL,
    Value                   DOUBLE  NOT NULL,

    PRIMARY KEY (DeviceID,BurstMessage,CommandNumber)
);

/* COMMANDS SECTION */
CREATE TABLE Commands
(
    CommandID               INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    DeviceID                INTEGER NOT NULL,
    CommandCode             INTEGER NOT NULL,
    CommandStatus           INTEGER NOT NULL,                           /*
                                                                         *  0 - new
                                                                         *  1 - sent
                                                                         *  2 - responded
                                                                         *  3 - failed
                                                                         */
    TimePosted              DATETIME NOT NULL,
    TimeResponsed           DATETIME,
    ErrorCode               INTEGER NOT NULL DEFAULT 0,
    ErrorReason             VARCHAR(50),
    Response                VARCHAR(50),
    Generated               INTEGER DEFAULT 0 NOT NULL,                 /*
                                                                         *  0 - manual
                                                                         *  1 - automatic
                                                                         */
    ParametersDescription   VARCHAR(1024) NULL
);
CREATE INDEX IX_Commands ON Commands( DeviceID, CommandCode, TimePosted);

CREATE TABLE CommandParameters
(
    CommandID               INTEGER NOT NULL,
    ParameterCode           INTEGER NOT NULL,
    ParameterValue          VARCHAR(50)
);
CREATE INDEX IX_CommandParameters ON CommandParameters( CommandID, ParameterCode);

CREATE TABLE CommandSet
(
    CommandCode             INTEGER NOT NULL,
    CommandName             VARCHAR(50) NOT NULL,
    ParameterCode           INTEGER NOT NULL DEFAULT -1,
    ParameterDescription    VARCHAR(100) NULL,
    IsVisible               INTEGER DEFAULT 1,

    PRIMARY KEY (CommandCode, ParameterCode)
);

/* VERSION SECTION */
-- Key must be enclosed between `` because is it considered a keyword
CREATE TABLE Properties
(
    `Key`               VARCHAR(50) NOT NULL,
    Value               VARCHAR(50)
);
CREATE UNIQUE INDEX IX_Properties ON Properties(`Key`); 

/* WEB SECTION */
CREATE TABLE Users
(
    UserID              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    UserName            VARCHAR(50) NOT NULL,
    FirstName           VARCHAR(50) NOT NULL,
    LastName            VARCHAR(50) NOT NULL,
    Email               VARCHAR(50) NOT NULL,




    Password            VARCHAR(250) NOT NULL,
    Role                INTEGER
);

CREATE TABLE Companies
(
    CompanyID           INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    CompanyName         VARCHAR(250) NOT NULL,
    LogoFile            VARCHAR(100)
);

/* REPORTS SECTION */
CREATE TABLE GraphNeighbors
(
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    GraphID             INTEGER NOT NULL,
    NeighborIndex       INTEGER NOT NULL
);
CREATE UNIQUE INDEX IX_GraphNeighbors ON GraphNeighbors( DeviceID, PeerID, GraphID);

CREATE TABLE Routes
(
    RouteID             INTEGER NOT NULL,
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    GraphID             INTEGER NOT NULL,
    SourceRoute         INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (RouteID, DeviceID)
);

CREATE TABLE SourceRoutes
(
    DeviceID            INTEGER NOT NULL,
    RouteID             INTEGER NOT NULL,
    Devices             VARCHAR(64) NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (RouteID, DeviceID)
);

CREATE TABLE Services
(
    ServiceID           INTEGER NOT NULL,
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    ApplicationDomain   INTEGER NOT NULL,
    SourceFlag          INTEGER NOT NULL,
    SinkFlag            INTEGER NOT NULL,
    IntermittentFlag    INTEGER NOT NULL,
    Period              INTEGER NOT NULL,
    RouteID             INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (ServiceID, DeviceID)	
);

CREATE TABLE Superframes
(
    SuperframeID        INTEGER NOT NULL,
    DeviceID            INTEGER NOT NULL,
    NumberOfTimeSlots   INTEGER NOT NULL,
    Active              INTEGER NOT NULL,
    HandheldSuperframe  INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (SuperframeID, DeviceID)
);

CREATE TABLE DeviceScheduleLinks
(
    SuperframeID        INTEGER NOT NULL,
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    SlotIndex           INTEGER NOT NULL,
    ChannelOffset       INTEGER NOT NULL,
    Transmit            INTEGER NOT NULL,
    Receive             INTEGER NOT NULL,
    Shared              INTEGER NOT NULL,
    LinkType            INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (SuperframeID, DeviceID, PeerID, SlotIndex)
);

CREATE TABLE ReportDeviceHealth
(
    DeviceID            INTEGER NOT NULL,
    PowerStatus         INTEGER NOT NULL,
    Generated           INTEGER NOT NULL,
    Terminated          INTEGER	NOT NULL,
    DllFailures         INTEGER	NOT NULL,
    NlFailures          INTEGER NOT NULL,
    CrcErrors           INTEGER NOT NULL,
    NonceLost           INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (DeviceID)
);

CREATE TABLE ReportNeighborHealthList
(
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    ClockSource         INTEGER NOT NULL,
    RSL                 INTEGER NOT NULL,
    Transmissions       INTEGER NOT NULL,
    FailedTransmissions INTEGER NOT NULL,
    Receptions          INTEGER NOT NULL,
    Timestamp           DATETIME NOT NULL,

    PRIMARY KEY (DeviceID, PeerID)
);

CREATE TABLE Alarms
(
    AlarmID             INTEGER PRIMARY KEY AUTOINCREMENT,
    DeviceID            INTEGER NOT NULL,
    AlarmType           INTEGER NOT NULL,
    AlarmTime           DATETIME NOT NULL,
    PeerID_GraphID      INTEGER,
    MIC                 INTEGER
);
CREATE INDEX IX_Alarms ON Alarms(AlarmTime);

CREATE TABLE ReportNeighborSignalLevels
(
    DeviceID            INTEGER NOT NULL,
    PeerID              INTEGER NOT NULL,
    RSL                 INTEGER NOT NULL,
	
    PRIMARY KEY (DeviceID, PeerID)
);

CREATE TABLE Dashboard
(
    SlotNumber          INTEGER,
    DeviceID            INTEGER,
    ChannelNo           INTEGER,
    GaugeType           INTEGER,
    MinValue            REAL,
    MaxValue            REAL
);

CREATE TABLE DeviceSetPublishersLog
(
    DeviceID            INTEGER NOT NULL PRIMARY KEY,
    State               INTEGER,
    Error               VARCHAR(100),
    Message             VARCHAR(500)
);

