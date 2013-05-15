

/* WH command - it is issued only by Web */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 122, 0, datetime('now'), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02';

