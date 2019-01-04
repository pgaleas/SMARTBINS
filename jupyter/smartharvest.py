import sys, getopt
import pandas as pd
import numpy as np
from matplotlib.path import Path
import matplotlib.patches as patches

def main(argv):
   points_str = ''
   excludenode = ''
   sensor = ''
   try:
      opts, args = getopt.getopt(argv,"hs:p:n:",["sensor","points=","xnode="])
   except getopt.GetoptError:
      print 'test.py -s <sensor> -p <point1,point2,...> -n <exclude_node>'
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print 'test.py -s <sensor> -p <point1,point2,...> -n <exclude_node>'
         sys.exit()
      elif opt in ("-p", "--points"):
         points_str = arg
      elif opt in ("-n", "--xnode"):
         excludenode = arg
      elif opt in ("-s", "--sensor"):
         sensor = arg

   return sensor, points_str, excludenode


# CONCATENATE ALL DATAFRAMES
def concate_dataframes():

    df_node1_imu = pd.read_csv("N1/N1A03_a.csv", sep=';')
    df_node1_temp = pd.read_csv("N1/N1A03_b.csv", sep=';')

    df_node2_imu = pd.read_csv("N2/N2A03_a.csv", sep=';')
    df_node2_temp = pd.read_csv("N2/N2A03_b.csv", sep=';')

    df_node3_imu = pd.read_csv("N3/N3A03_a.csv", sep=';')
    df_node3_temp = pd.read_csv("N3/N3A03_b.csv", sep=';')

    df_node4_imu = pd.read_csv("N4/N4A03_a.csv", sep=';')
    df_node4_temp = pd.read_csv("N4/N4A03_b.csv", sep=';')

    df_node5_imu = pd.read_csv("N5/N5A03_a.csv", sep=';')
    df_node5_temp = pd.read_csv("N5/N5A03_b.csv", sep=';')

    df_imu = pd.concat([df_node1_imu, df_node2_imu,df_node3_imu,df_node4_imu,df_node5_imu], sort=False)
    df_temp = pd.concat([df_node1_temp,df_node2_temp,df_node3_temp,df_node4_temp,df_node5_temp], sort=False)

    df_imu['imu_vector'] = np.sqrt(pow(df_imu['acc_x']/16384*9.8,2) + pow(df_imu['acc_y']/16384*9.8,2) + pow(df_imu['acc_z']/16384*9.8-9.8,2))

    # Delete some columns
    df_imu = df_imu.drop(columns=['n_frame',' n_frame','acc_x','acc_y','acc_z','giro_x','giro_y','giro_z','imu_time'])
    df_temp = df_temp.drop(columns=['head_mark','n_frame','temp3','hum','batt_status','end_mark','time_rtc'])

    # Add columns to (imu_vector) df_temp
    df_temp['imu_vector'] = np.repeat(np.nan,len(df_temp))

    # Add columns (temp1, temp2, weight) to df_imu
    df_imu['temp1'] = np.repeat(np.nan,len(df_imu))
    df_imu['temp2'] = np.repeat(np.nan,len(df_imu))
    df_imu['weight'] = np.repeat(np.nan,len(df_imu))

    # Swap columns of df_temp
    cols_temp = df_temp.columns.tolist()
    cols_temp = [cols_temp[0]] + cols_temp[4:] + cols_temp[1:4]
    df_temp = df_temp[cols_temp]


    return df_imu, df_temp

# PRINT VERTICAL or HORIZONTAL LINES ON THE GRAPH AT DEFINED POSITIONS
def print_harvest_lines(subplot_obj, lines_x, mode):
    subplot_ymin, subplot_ymax = subplot_obj.get_ylim()
    lenght_lines = len(lines_x)
    max_lines = np.amax(lines_x)
    min_lines = np.amin(lines_x)

    verts = []
    codes = []
    verts_t = []
    codes_t = []

    if(mode == 'vertical' or mode == 'all'):
        """verts = [
        (lines_x[0], subplot_ymin), (lines_x[0], subplot_ymax),
        (lines_x[1], subplot_ymin), (lines_x[1], subplot_ymax),
        (lines_x[2], subplot_ymin), (lines_x[2], subplot_ymax),
        (lines_x[3], subplot_ymin), (lines_x[3], subplot_ymax),
        (lines_x[4], subplot_ymin), (lines_x[4], subplot_ymax),
        (lines_x[5], subplot_ymin), (lines_x[5], subplot_ymax),
        (lines_x[6], subplot_ymin), (lines_x[6], subplot_ymax),
        ]"""
        for i in range(len(lines_x)-1):
        	verts.append((lines_x[i], subplot_ymin))
        	verts.append((lines_x[i], subplot_ymax))


        """codes = [
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        ]"""

        for j in range(len(lines_x)-1):
            codes.append(Path.MOVETO)
            codes.append(Path.LINETO)

    if(mode == 'horizontal' or mode == 'all'):
        verts_t = [
        (min_lines, 1), (max_lines, 1),
        (min_lines, 2), (max_lines, 2),
        (min_lines, 3), (max_lines, 3),
        (min_lines, 4), (max_lines, 4),
        (min_lines, 5), (max_lines, 5),
        (min_lines, 6), (max_lines, 6),
        ]

        codes_t = [
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        Path.MOVETO, Path.LINETO,
        ]

        verts = verts + verts_t
        codes = codes + codes_t

    path = Path(verts, codes)
    patch = patches.PathPatch(path, facecolor='black', lw=0.5, ls='dashed')
    return patch

# CALCULATE THE MEAN OF THE IMU_VECTOR BETWEEN TWO POINTS
def getImuMean(nodeNr, x1, x2, df):
	sensor = 'imu'
	node_name = "n"+ str(nodeNr)
	imu_mean = np.mean(df[(df.base_time > x1) & (df.base_time < x2) & (df.id_device == nodeNr)]['imu_vector'].values)
	return imu_mean

# CALCULATE THE SLOPE OF THE WEIGHT CURVE BETWEEN TWO POINTS
def getWeightSlope(nodeNr, x1, x2, calibration, df):

    node_name = "n"+ str(nodeNr)

    # Calculate valid y-axis values for the points in the x-axis
    while True:
    	y_1 = df[(df.base_time == x1) & (df.id_device == nodeNr)]['weight'].values
    	# if the point x does not exist in the data
    	if(len(y_1) == 0) :
    		if (x1 > 0): x1 = x1-1
    		else: x1 = x1+1
    	else:
    		break

    while True:
    	y_2 = df[(df.base_time == x2) & (df.id_device == nodeNr)]['weight'].values
    	# if the point x does not exist in the data
    	if(len(y_2) == 0) :
    		if (x2 > 0): x2 = x2-1
    		else: x2 = x2+1
    	else:
    		break

    # Calibrate the Y values
    weight_zero = calibration.loc[node_name,'weight_zero']
    weight_calibration = calibration.loc[node_name,'weight_calibration']

    # print("weight_zero,weight_calibration = ("+str(weight_zero)+","+str(weight_calibration)+")")
    y1 = (y_1[0]-weight_zero)/weight_calibration
    y2 = (y_2[0]-weight_zero)/weight_calibration

    # Calculate slope of weight curve for harvesting
    slope = (float)(y2-y1)/(x2-x1)

    return slope

# CALCULATE THE WEIGHT VALUE IN THE MIDDLE OF TWO POINTS
def getWeight(nodeNr, x1, x2, calibration, df):

	node_name = "n"+ str(nodeNr)

	# Get middle point
	x = x1 + int((x2 - x1)/2)

	# Calculate valid y-axis values for the points in the x-axis
	while True:
		y = df[(df.base_time == x) & (df.id_device == nodeNr)]['weight'].values
		# if the point x does not exist in the data
		if(len(y) == 0) :
			if(x>0): x = x-1
			else: x = x+1
		else:
			break

	# Calibrate the Y values
	weight_zero = calibration.loc[node_name,'weight_zero']
	weight_calibration = calibration.loc[node_name,'weight_calibration']
	weight = (y[0]-weight_zero)/weight_calibration

	return weight

# CALCULATE THE TEMPERATURE VALUE IN THE MIDDLE OF TWO POINTS
def getTemperature(sensor, nodeNr, x1, x2, calibration, df):

	node_name = "n"+ str(nodeNr)

	# Get middle point
	x = x1 + int((x2 - x1)/2)

	# Calculate valid y-axis values for the points in the x-axis
	while True:
		y = df[(df.base_time == x) & (df.id_device == nodeNr)][sensor].values
		# if the point x does not exist in the data
		if(len(y) == 0) :
			if(x>0): x = x-1
			else: x = x+1
		else:
			break

	temp_calibration = calibration.loc[node_name,'temp_calibration']
	temperature = y[0]/temp_calibration

	return temperature

# CALCULATE THE MEAN TEMPERATURE VALUE BETWEEN TWO POINTS
def getTempMean(sensor, nodeNr, x1, x2, calibration, df):

	node_name = "n"+ str(nodeNr)

	# Calculate valid y-axis values for the points in the x-axis
	while True:
		y_1 = df[(df.base_time == x1) & (df.id_device == nodeNr)][sensor].values
		# if the point x does not exist in the data
		if(len(y_1) == 0) :
			if(x1>0): x1 = x1-1
			else: x1 = x1+1
		else:
			break

	while True:
		y_2 = df[(df.base_time == x2) & (df.id_device == nodeNr)][sensor].values
		# if the point x does not exist in the data
		if(len(y_2) == 0) :
			if(x2>0): x2 = x2-1
			else: x2 = x2+1
		else:
			break

	temp_calibration = calibration.loc[node_name,'temp_calibration']
	df_selected = df[(df.base_time > x1) & (df.base_time < x2) & (df.id_device == nodeNr)][sensor].values/temp_calibration

	temp_mean = np.mean(df_selected)

	return temp_mean
