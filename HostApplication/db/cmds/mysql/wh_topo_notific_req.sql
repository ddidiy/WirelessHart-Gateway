USE Monitor_Host_17;


/* WH subscribe for topo_notification command - it is issued only by HostApp */
INSERT INTO Commands (DeviceID, CommandCode, CommandStatus, TimePosted, ErrorCode)
SELECT DeviceID, 9, 0, Now(), 0
FROM Devices WHERE Address64 = '00-1B-1E-F9-81-00-00-02'; /*it must be gw's address*/

