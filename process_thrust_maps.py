
# ----------------------------------------------------------------------
#   Imports
# ----------------------------------------------------------------------

import numpy as np
import pylab as plt
import VyPy

import glob, os, shutil, sys

import matplotlib
matplotlib.rc('axes', labelsize='large')


"""
todo:
thrust coefficient
torque coeffiecient
power coefficient
advance ratio (normalizes rpm and forward flight)


"""

# ----------------------------------------------------------------------
#   Main
# ----------------------------------------------------------------------

def main():
    
    foldername = 'dyno - 20140603'
    filenames = glob.glob(foldername + '/*.txt')
    
    for filename in filenames:
        
        data = load_data(filename)
        
        data = process_data(data)
        
        plot_data(data)
        
        
    
    return

def process_data(data):
    
    # unpack
    rpm = data.RPM
    thr = data.Throttle
    T   = data.thrustAvg
    Q   = data.torqueAvg
    I   = data.currentAvg
    V   = data.voltageAvg
    esc = data.esc_type
    mot = data.motor_type
    dia = data.prop_params.diameter
    A   = np.pi * dia**2. / 4
    rho = 1.225 # kg/m^3
    
    # process
    omega = rpm * np.pi / 30.
    # rot / min * 1min/60sec * 2pi rad / 1 rot
    
    P_E = V * I
    P_S = Q * omega
    P_A = T * np.sqrt(T/A * 0.5 / rho)
    
    eff_shaft = P_S / P_E * 100.    
    eff_prop = P_A/P_S * 100
    
    eff_shaft[eff_shaft < 0.] = 0.
    eff_prop[eff_prop < 0.] = 0.
    eff_shaft[np.abs(eff_shaft) == np.inf] = 0.
    eff_prop[np.abs(eff_prop) == np.inf] = 0.    
    
    # pack
    data.power_electric = P_E
    data.power_shaft    = P_S
    data.power_prop     = P_A
    
    data.efficiency_shaft = eff_shaft
    data.efficiency_prop  = eff_prop
    
    return data

def plot_data(data):
    
    # unpack
    rpm = data.RPM
    thr = data.Throttle
    T   = data.thrustAvg
    Q   = data.torqueAvg
    I   = data.currentAvg
    V   = data.voltageAvg
    Pe  = data.power_electric
    Ps  = data.power_shaft
    Pp  = data.power_prop
    es  = data.efficiency_shaft
    ep  = data.efficiency_prop
    
    bat = data.battery_type
    esc = data.esc_type
    mot = data.motor_type
    prp = data.prop_type
    
    fig = plt.figure(figsize=(8,10))
    
    xlims = [0,9000]
    
    # Throttle
    ax1 = plt.subplot(4,1,1)
    
    ax1.plot(rpm, thr, 'ko-')
    
    plt.xlim(xlims)    
    plt.ylim([0,100])

    #ax1.set_xlabel('RPM')
    ax1.set_ylabel('Throttle (%)')
    ax1.set_title('Batt: %s, ESC: %s, Motor: %s, Prop: %s' % (bat,esc,mot,prp))
    
    # Thrust, Torque
    ax1 = plt.subplot(4,1,2)
    
    ax1.plot(rpm, T, 'bo-')
        
    plt.xlim(xlims)    
    plt.ylim([0,12])
    
    #ax1.set_xlabel('RPM')
    ax1.set_ylabel('Thrust (N)', color='b')    
    for tl in ax1.get_yticklabels():
        tl.set_color('b')
    
    ax2 = ax1.twinx()
    
    ax2.plot(rpm, Q, 'ro-')
    
    ax2.set_ylabel('Torque (Nm)', color='r')
    
    plt.xlim(xlims)      
    plt.ylim([0,0.16])    
    
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
        
    # Current, Voltage
    ax1 = plt.subplot(4,1,3)
    
    ax1.plot(rpm, I, 'bo-')
    
    plt.xlim(xlims)      
    plt.ylim([0,16])            
    
    #ax1.set_xlabel('RPM')
    ax1.set_ylabel('Current (A)', color='b')
    
    for tl in ax1.get_yticklabels():
        tl.set_color('b')
    
    ax2 = ax1.twinx()
    
    ax2.plot(rpm, V, 'ro-')
    
    plt.xlim(xlims)      
    plt.ylim([10.8,15.6])            
    
    ax2.set_ylabel('Voltage (V)', color='r')
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
        
    # Power, Efficiency
    ax1 = plt.subplot(4,1,4)
    
    ax1.plot(rpm, Pe, 'bo-')
    ax1.plot(rpm, Ps, 'bs-')
    ax1.plot(rpm, Pp, 'b^-')
    
    plt.xlim(xlims)      
    plt.ylim([0,200])              
    
    ax1.set_xlabel('RPM')
    ax1.set_ylabel('Power (W)', color='b')
    for tl in ax1.get_yticklabels():
        tl.set_color('b')


    pe = ax1.plot([0], [np.nan], 'ko-', label='Electric')
    ps = ax1.plot([0], [np.nan], 'ks-', label='Shaft')
    pp = ax1.plot([0], [np.nan], 'k^-', label='Propeller')
    
    lg = plt.legend(loc='upper left')
    lg.draw_frame(False)
    
    ax2 = ax1.twinx()
    
    ax2.plot(rpm, es, 'rs-')
    ax2.plot(rpm, ep, 'r^-')
    
    plt.xlim(xlims)      
    plt.ylim([0,80])              
    
    ax2.set_ylabel('Net Efficiency (%)', color='r')
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
    
    # save    
    folder = os.path.split(data.filename)[0]
    filename = '%s - %s - %s - %s - %s' %(data.test_type,bat,esc,mot,prp)
    filename = folder + '/' + filename + '.png'
    plt.savefig(filename,dpi=200)
    
    #plt.show(block=True)
    plt.close()
        

prop_parameters = VyPy.data.obunch({
    'dji 1038': { 'diameter': 10. * 0.0254 ,
                  'pitch'   : 3.8 * 0.0254 } , 
    'dji 1147': { 'diameter': 11. * 0.0254 ,
                  'pitch'   : 4.7 * 0.0254 } ,     
    'apc 1047': { 'diameter': 10. * 0.0254 ,
                  'pitch'   : 4.7 * 0.0254 } ,     
    'apc 1147': { 'diameter': 11. * 0.0254 ,
                  'pitch'   : 4.7 * 0.0254 } ,     
})

def load_data(filename):
    
    data = VyPy.data.obunch()
    
    # process the filename 
    name = os.path.split(filename)[-1]
    name = os.path.splitext(name)[0]
    name = name.split('_')
    
    data.filename     = filename
    data.test_type    = ' '.join( name[0:1+1] )
    data.timestamp    = ' '.join( name[2:2+1] )
    data.battery_type = ' '.join( name[3:3+1] )
    data.esc_type     = ' '.join( name[4:5+1] )
    data.motor_type   = ' '.join( name[6:7+1] )
    data.prop_type    = ' '.join( name[8:9+1] )
    data.prop_params  = prop_parameters[data.prop_type]
    
    # load the data
    for i,line in enumerate( file(filename).readlines() ):
        
        if i == 0:
            headers = ''.join(line.split('"')).split()
            for header in headers:
                data[header] = []
            continue
        elif i == 1:
            continue
        
        else:
            values = ''.join(line.split('"')).split()
            values = map(float,values)
            
            for j,header in enumerate( headers ):
                data[header].append( values[j] )
            
    for header in headers:
        data[header] = np.array( data[header] )
    
    # thrust kg-f -> N
    data['thrustAvg'] = data['thrustAvg'] * 9.81
    
    return data


# ----------------------------------------------------------------------
#   Call Main
# ----------------------------------------------------------------------

if __name__ == '__main__':
    main()






