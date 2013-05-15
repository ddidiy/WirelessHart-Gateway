

/* WH unsubscribe for notification command - it is issued only by HostApp */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 3, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '001B:1EF8:7000:0703';

