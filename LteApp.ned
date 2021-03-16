import inet.applications.contract.IUDPApp;

simple LteApp like IUDPApp
{
    parameters:
        int localPort = default(9330);
        int mcastPort = default(9331);
        string mcastAddress = default("224.0.0.10");
        string middlewareModule;
        string interfaceTableModule;

    gates:
        output udpOut;
        input udpIn;
}
