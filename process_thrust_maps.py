
# ----------------------------------------------------------------------
#   Imports
# ----------------------------------------------------------------------

import matplotlib
matplotlib.use('Agg')
import pylab as plt
matplotlib.rc('axes', labelsize='large')

import glob, os, shutil, sys
import yaml

import numpy as np
import VyPy




import VyPy.tools.pint
from VyPy.tools.pint.Units import Units



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
    
    foldername = 'test_data/sess10'
    filenames = glob.glob(foldername + '/*.txt')
    
    for filename in filenames:
        
        data = load_raw_data(filename)
        
        data = process_data(data)
        
        plot_data(data)
        
        write_csv(data)
    
    return

def process_data(data):
    
    # unpack
    rpm = data.mean.RPM
    thr = data.mean.Throttle
    T   = data.mean.Thrust
    Q   = data.mean.Torque
    I   = data.mean.Current
    V   = data.mean.Voltage

    dia = data.meta.Prop.Diameter
    dia,dia_units = [a.strip() for a in dia.split()]
    dia = float(dia)
    
    # units to SI
    T = T * Units[data.meta.Units['Thrust']]
    if 'g' in data.meta.Units['Thrust']:
        T *= 9.81
    
    torq_units = data.meta.Units['Torque'].split('-')
    Q = Q * Units[torq_units[0]] * Units[torq_units[1]]
    if 'g' in torq_units[0]:
        Q *= 9.81
    
    I = I * Units[data.meta.Units['Current']]
    V = V * Units[data.meta.Units['Voltage']]
    dia = dia * Units[dia_units]
    
    A   = np.pi * dia**2. / 4
    rho = 1.225 # kg/m^3    
    
    # process
    omega = rpm * np.pi / 30.
    # rot / min * 1min/60sec * 2pi rad / 1 rot
    
    P_E = V * I
    P_S = Q * omega
    #P_A = T * np.sqrt(T/A * 0.5 / rho)
    P_A = ( T**3. / (4.*rho*A) ) ** 0.5
    P_A[omega==0] = 0
    
    eff_shaft = P_S / P_E * 100.    
    eff_prop = P_A/P_S * 100
    
    eff_shaft[omega==0] = np.nan
    eff_prop[omega==0] = np.nan
    
    eff_shaft[eff_shaft < 0.] = 0.
    eff_prop[eff_prop < 0.] = 0.
    eff_shaft[np.abs(eff_shaft) == np.inf] = 0.
    eff_prop[np.abs(eff_prop) == np.inf] = 0.
    
    # pack
    data.mean.Power = VyPy.data.obunch()
    data.mean.Power.to_ESC    = P_E
    data.mean.Power.to_Shaft  = P_S
    data.mean.Power.to_Thrust = P_A
    
    data.mean.Efficiency = VyPy.data.obunch()
    data.mean.Efficiency.Battery_to_Shaft     = eff_shaft
    data.mean.Efficiency.Shaft_to_Thrust  = eff_prop
    
    return data

def write_csv(data):

    # unpack
    rpm = data.mean.RPM
    thr = data.mean.Throttle
    T   = data.mean.Thrust
    Q   = data.mean.Torque
    I   = data.mean.Current
    V   = data.mean.Voltage
    Pe  = data.mean.Power.to_ESC 
    Ps  = data.mean.Power.to_Shaft
    Pp  = data.mean.Power.to_Thrust
    es  = data.mean.Efficiency.Battery_to_Shaft
    ep  = data.mean.Efficiency.Shaft_to_Thrust
    
    bat = data.meta.Battery
    esc = data.meta.ESC
    mot = data.meta.Motor
    prp = data.meta.Prop.Name    
    
    
    filename = os.path.splitext( data.meta.Filename )[0] + '.csv'
    fileout = open(filename,'w')
    
    title = 'Batt: %s, ESC: %s, Motor: %s, Prop: %s' % (bat,esc,mot,prp)
    fileout.write(title + '\n')
    
    keys = [
        'Throttle [%]',
        'RPM',
        'Thrust [%s]' % data.meta.Units['Thrust'],
        'Torque [%s]' % data.meta.Units['Torque'],
        'Voltage [%s]' % data.meta.Units['Voltage'],
        'Current [%s]' % data.meta.Units['Current'],
        'Power_to_ESC [W]',
        'Power_to_Shaft [W]',
        'Power_to_Thrust [W]',
        'Efficiency-Battery_to_Shaft [%]',
        'Efficiency-Shaft_to_Thrust [%]',
    ]
    
    values = [
        thr,
        rpm,
        T,
        Q,
        V,
        I,
        Pe,
        Ps,
        Pp,
        es,
        ep,
    ]
    
    line = ', '.join(keys)
    fileout.write(line+'\n')
    
    for line in zip(*values):
        line = ', '.join(map(str,line))
        fileout.write(line+'\n')
    
    fileout.close()
    


def plot_data(data):
    
    # unpack
    rpm = data.mean.RPM
    thr = data.mean.Throttle
    T   = data.mean.Thrust
    Q   = data.mean.Torque
    I   = data.mean.Current
    V   = data.mean.Voltage
    Pe  = data.mean.Power.to_ESC 
    Ps  = data.mean.Power.to_Shaft
    Pp  = data.mean.Power.to_Thrust
    es  = data.mean.Efficiency.Battery_to_Shaft
    ep  = data.mean.Efficiency.Shaft_to_Thrust
    
    bat = data.meta.Battery
    esc = data.meta.ESC
    mot = data.meta.Motor
    prp = data.meta.Prop.Name
    
    fig = plt.figure(figsize=(8,10))
    
    # RPM
    ax1 = plt.subplot(4,1,1)
    
    ax1.plot(thr, rpm, 'ko-')
    
    plt.xlim([-5,105])    
    plt.ylim([0, max(10000,np.max(rpm))])

    ax1.set_ylabel('RPM')
    ax1.set_title('Batt: %s, ESC: %s, \nMotor: %s, Prop: %s' % (bat,esc,mot,prp))
    
    # Thrust, Torque
    ax1 = plt.subplot(4,1,2)
    
    ax1.plot(thr, T, 'bo-')
        
    plt.xlim([-5,105])    
    plt.ylim([0,2000.])
    
    ax1.set_ylabel('Thrust (%s)' % data.meta.Units['Thrust'], color='b')    
    for tl in ax1.get_yticklabels():
        tl.set_color('b')
    
    ax2 = ax1.twinx()
    
    ax2.plot(thr, Q, 'ro-')
    
    ax2.set_ylabel('Torque (%s)' % data.meta.Units['Torque'], color='r')
    
    plt.xlim([-5,105])      
    plt.ylim([0,45])    
    
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
    
    # Voltage
    ax1 = plt.subplot(4,1,3)
    
    ax1.plot(thr, V, 'bo-')
    
    plt.xlim([-5,105])      
    plt.ylim([14.0,17.0])            
    
    ax1.set_ylabel('Voltage (V)', color='b')
    for tl in ax1.get_yticklabels():
        tl.set_color('b')        
        
    # Current
    ax2 = ax1.twinx()
    
    ax2.plot(thr, I, 'ro-')
    
    plt.xlim([-5,105])      
    plt.ylim([0,16])            
    
    ax2.set_ylabel('Current (A)', color='r')
    
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
    

        
    # Power, Efficiency
    ax1 = plt.subplot(4,1,4)
    
    ax1.plot(thr, Pe, 'bo-')
    ax1.plot(thr, Ps, 'bs-')
    ax1.plot(thr, Pp, 'b^-')
    
    plt.xlim([-5,105])      
    plt.ylim([0,200])              
    
    ax1.set_xlabel('Throttle [%]')
    ax1.set_ylabel('Power (W)', color='b')
    for tl in ax1.get_yticklabels():
        tl.set_color('b')


    pe = ax1.plot([0], [np.nan], 'ko-', label='to ESC')
    ps = ax1.plot([0], [np.nan], 'ks-', label='to Shaft')
    pp = ax1.plot([0], [np.nan], 'k^-', label='to Thrust')
    
    lg = plt.legend(loc='upper left')
    lg.draw_frame(False)
    
    ax2 = ax1.twinx()
    
    ax2.plot(thr, es*0.0+100., 'ro-')
    ax2.plot(thr, es, 'rs-')
    ax2.plot(thr, es*ep/100., 'r^-')
    
    plt.xlim([-5,105])      
    plt.ylim([-5,105])              
    
    ax2.set_ylabel('Net Efficiency (%)', color='r')
    for tl in ax2.get_yticklabels():
        tl.set_color('r')
    
    # save    
    #folder = os.path.split(data.meta.Filename)[0]
    #filename = 'Dyno Data -- %s - %s - %s - %s' %(bat,esc,mot,prp)
    #filename = folder + '/' + filename + '.png'
    filename = os.path.splitext( data.meta.Filename )[0] + '.png'
    plt.savefig(filename,dpi=200)
    
    #plt.show(block=True)
    plt.close()
    


def load_raw_data(filename):
    
    meta = VyPy.data.obunch()
    raw  = VyPy.data.obunch()
    
    for k in ['Date','Motor','Battery','ESC','Prop','Notes','Filename']:
        meta[k] = None
    meta.Units = VyPy.data.obunch()
        
    file_in = open(filename,'r')
    
    # read meta data
    meta_in = ''
    while True:
        line = file_in.readline()
        if line.strip():
            meta_in += line
        elif len(meta_in):
            break
    
    loaded = yaml.safe_load(meta_in)
    meta.update(loaded)
    meta.Filename = filename
    
    for k,v in meta.items():
        if isinstance(v,dict):
            meta[k] = VyPy.data.obunch(v)
    
    
    # read raw data
    nlines = 0
    for line in file_in.readlines():
        
        if not line.strip():
            continue
        
        line = line.split(',')
        for item in line:
            
            item = item.split(':')
            key = item[0].strip()
            
            item = item[1].strip().split(' ')
            value = float(item[0])
            
            if len(item)==2:
                units = item[1]
            else:
                units = '-'
            
            meta.Units[key] = units
            
            if not raw.has_key(key):
                raw[key] = [np.nan] * nlines
            
            raw[key].append(value)
        
        nlines += 1

    file_in.close()
    
    # average at each throttle
    array = np.array( zip(*raw.values()) )
    
    dtypes = [(k,'f8') for k in raw.keys()] 
    raw = array.view(dtype=dtypes).view(np.recarray)
    
    throttle = raw.Throttle
    throttle = np.unique(throttle)
    
    mean = np.zeros([len(throttle), array.shape[1]])
    for i,t in enumerate(throttle):
        it = (raw.Throttle == t)[:,0]
        mean[i,:] = np.nanmean(array[it,:],axis=0)
    
    temp = VyPy.data.obunch()
    for i,k in enumerate(meta.Units.keys()):
        temp[k] = mean[:,i]
    mean = temp
    
    if np.mean(mean.Thrust) < 0: mean.Thrust *= -1.
    if np.mean(mean.Torque) < 0: mean.Torque *= -1.
    
    data = VyPy.data.obunch()
    data.meta = meta
    data.mean = mean
    data.raw  = raw
    
    return data
        

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






