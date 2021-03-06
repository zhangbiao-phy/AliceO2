<!-- doxy
\page refDetectorsMUONMCHConditions Conditions
/doxy -->

# MCH Conditions

## DCS to CCDB

To test the DCS to CCDB route you can use the following 3 parts worfklow pipeline : 

```shell
o2-calibration-mch-dcs-sim-workflow --max-timeframes 600 --max-cycles-no-full-map 10 -b | \
o2-calibration-mch-dcs-processor-workflow --hv-max-size 0 --hv-max-duration 300 -b | \
o2-calibration-ccdb-populator-workflow --ccdb-path="http://localhost:6464" -b
```

- `o2-calibration-mch-dcs-sim-worfklow` is just generating fake random MCH DCS data points, 
- `o2-calibration-mch-dcs-processor-workflow` gathers the received data points into a container object 
- `o2-calibration-ccdb-populator-workflow` uploads the container object to the CCDB (in this example a local dev ccdb).

 The container object that groups the datapoints is considered ready to be shipped either when the data points span a long enough duration (see the `--hv-max-duration` and `--hv-max-duration` options of the `o2-calibration-mch-dcs-processor-workflow`) or is big enough (see the `--lv-max-size` and `--hv-max-size` options).

## DCS Data Points

### HV

The MCH high voltage (HV) system is composed of 188 channels :

- 48 channels for stations 1 and 2 (3 HV channel per quadrant x 16 quadrants)
- 140 channels for stations 3, 4, 5 (1 HV channel per slat x 140 slats)

### LV

The MCH low voltage (LV) system is composed of 328 channels :

- 216 channels (108 x 2 different voltage values) to power up the front-end
  electronics (dualsampas)
- 112 channels to power up the readout crates hosting the solar (readout) cards

## CCDB quick check

Besides the web browsing of the CCDB, another quick check can be performed with 
 the `o2-mch-dcs-ccdb` program to dump the DCS datapoints (hv, lv, or both) or 
 the datapoint config valid at a given timestamp.

```
$ o2-mch-dcs-ccdb --help
$ o2-mch-dcs-ccdb --ccdb http://localhost:6464 --query hv --query lv --query dpconf
```

The same program can be used to upload to CCDB the DCS data point configuration
 for the dcs-proxy : 

```
$ o2-mch-dcs-ccdb --put-datapoint-config --ccdb http://localhost:8080
```

