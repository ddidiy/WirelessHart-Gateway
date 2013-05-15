USE Monitor_Host_17;

SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;
SET AUTOCOMMIT=0;

START TRANSACTION;

/* WH read value command - it is issued only by Web */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 1, 0, Now(), 0
FROM Devices WHERE Address64 = '001B:1EF8:7000:0703';

/*channelID*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
VALUES(last_insert_id(), 10, 3);

/*bypassIOCache - optional*/
INSERT INTO CommandParameters (CommandID, ParameterCode, ParameterValue)
VALUES(last_insert_id(), 1300, 0);


COMMIT;
SET AUTOCOMMIT=1;