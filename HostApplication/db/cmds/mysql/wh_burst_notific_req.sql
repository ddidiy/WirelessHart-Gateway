USE Monitor_Host_17;


/* WH subscribe for notification command - it is issued only by HostApp */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 3, 0, Now(), 0
FROM Devices WHERE Address64 = '001B:1EF8:7000:0703';

