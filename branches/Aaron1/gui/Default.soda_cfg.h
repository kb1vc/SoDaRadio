  config_stream << "<?xml version='1.0' encoding='utf-8'?>";
  config_stream << "<SoDaRadio>";
  config_stream << "	<af>";
  config_stream << "		<gain>13</gain>";
  config_stream << "		<bw>3</bw>";
  config_stream << "	</af>";
  config_stream << "	<rx>";
  config_stream << "		<mode>NBFM</mode>";
  config_stream << "		<freq>162399777.7777778</freq>";
  config_stream << "		<prev_freq>10368291722</prev_freq>";
  config_stream << "	</rx>";
  config_stream << "	<tx>";
  config_stream << "		<freq>162300000</freq>";
  config_stream << "		<prev_freq>0</prev_freq>";
  config_stream << "		<rf_outpower>21.4</rf_outpower>";
  config_stream << "		<tx_rx_locked>false</tx_rx_locked>";
  config_stream << "	</tx>";
  config_stream << "	<station>";
  config_stream << "		<call/>";
  config_stream << "		<qth/>";
  config_stream << "	</station>";
  config_stream << "	<cw>";
  config_stream << "		<speed>10</speed>";
  config_stream << "		<sidetone>20</sidetone>";
  config_stream << "	</cw>";
  config_stream << "	<spectrum>";
  config_stream << "		<center_freq>162400000</center_freq>";
  config_stream << "		<yscale>10</yscale>";
  config_stream << "		<bandspread>200000</bandspread>";
  config_stream << "		<reflevel>0</reflevel>";
  config_stream << "		<display>waterfall</display>";
  config_stream << "	</spectrum>";
  config_stream << "	<reference>";
  config_stream << "		<source>true</source>";
  config_stream << "	</reference>";
  config_stream << "	<current_band>WX</current_band>";
  config_stream << "	<bands>";
  config_stream << "		<band>";
  config_stream << "			<name>6m</name>";
  config_stream << "			<upper_band_edge>54</upper_band_edge>";
  config_stream << "			<lower_band_edge>50</lower_band_edge>";
  config_stream << "			<last_rx_freq>50</last_rx_freq>";
  config_stream << "			<last_tx_freq>50</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>1</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>0</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>0</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>0</outpower>";
  config_stream << "				<rx_locked>true</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>2m</name>";
  config_stream << "			<upper_band_edge>148</upper_band_edge>";
  config_stream << "			<lower_band_edge>144</lower_band_edge>";
  config_stream << "			<last_rx_freq>144.2959444444444</last_rx_freq>";
  config_stream << "			<last_tx_freq>144.2910555555555</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>1</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>11</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>171</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>23.8</outpower>";
  config_stream << "				<rx_locked>false</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>1.25m</name>";
  config_stream << "			<upper_band_edge>225</upper_band_edge>";
  config_stream << "			<lower_band_edge>219</lower_band_edge>";
  config_stream << "			<last_rx_freq>219</last_rx_freq>";
  config_stream << "			<last_tx_freq>219</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>2</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>0</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>0</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>0</outpower>";
  config_stream << "				<rx_locked>true</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>70cm</name>";
  config_stream << "			<upper_band_edge>450</upper_band_edge>";
  config_stream << "			<lower_band_edge>420</lower_band_edge>";
  config_stream << "			<last_rx_freq>420</last_rx_freq>";
  config_stream << "			<last_tx_freq>420</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>2</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>0</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>0</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>0</outpower>";
  config_stream << "				<rx_locked>true</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>33cm</name>";
  config_stream << "			<upper_band_edge>928</upper_band_edge>";
  config_stream << "			<lower_band_edge>902</lower_band_edge>";
  config_stream << "			<last_rx_freq>902</last_rx_freq>";
  config_stream << "			<last_tx_freq>902</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>2</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>0</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>0</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>0</outpower>";
  config_stream << "				<rx_locked>true</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>23cm</name>";
  config_stream << "			<upper_band_edge>1300</upper_band_edge>";
  config_stream << "			<lower_band_edge>1240</lower_band_edge>";
  config_stream << "			<last_rx_freq>1240</last_rx_freq>";
  config_stream << "			<last_tx_freq>1240</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>2</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>0</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>0</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>0</outpower>";
  config_stream << "				<rx_locked>true</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>10GHz</name>";
  config_stream << "			<upper_band_edge>10370.5</upper_band_edge>";
  config_stream << "			<lower_band_edge>10360</lower_band_edge>";
  config_stream << "			<last_rx_freq>10368.09572222222</last_rx_freq>";
  config_stream << "			<last_tx_freq>10368.09572222222</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>3</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>9</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>148</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>21.4</outpower>";
  config_stream << "				<rx_locked>false</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>true</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "			<transverter_lo_freq>1136</transverter_lo_freq>";
  config_stream << "			<transverter_multiplier>9</transverter_multiplier>";
  config_stream << "			<low_side_injection>false</low_side_injection>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>10GHz-B210</name>";
  config_stream << "			<upper_band_edge>10370</upper_band_edge>";
  config_stream << "			<lower_band_edge>10360</lower_band_edge>";
  config_stream << "			<last_rx_freq>10368.09572222222</last_rx_freq>";
  config_stream << "			<last_tx_freq>10368.09572222222</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>TX/RX</rx_antenna_choice>";
  config_stream << "			<enable_transmit>true</enable_transmit>";
  config_stream << "			<default_mode>CW_U</default_mode>";
  config_stream << "			<band_id>3</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>9</gain>";
  config_stream << "				<bw>2</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>148</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>21.4</outpower>";
  config_stream << "				<rx_locked>false</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>true</transverter_mode>";
  config_stream << "			<transverter_lo_freq>5180</transverter_lo_freq>";
  config_stream << "			<transverter_multiplier>1</transverter_multiplier>";
  config_stream << "			<transverter_local_lo>true</transverter_local_lo>";
  config_stream << "			<low_side_injection>false</low_side_injection>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>FM Broadcast</name>";
  config_stream << "			<upper_band_edge>108</upper_band_edge>";
  config_stream << "			<lower_band_edge>87</lower_band_edge>";
  config_stream << "			<last_rx_freq>88.5</last_rx_freq>";
  config_stream << "			<last_tx_freq>87</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>false</enable_transmit>";
  config_stream << "			<default_mode>WBFM</default_mode>";
  config_stream << "			<band_id>2</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>14</gain>";
  config_stream << "				<bw>3</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>209</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>21.4</outpower>";
  config_stream << "				<rx_locked>false</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "		<band>";
  config_stream << "			<name>WX</name>";
  config_stream << "			<upper_band_edge>162.65</upper_band_edge>";
  config_stream << "			<lower_band_edge>162.3</lower_band_edge>";
  config_stream << "			<last_rx_freq>162.3997777777778</last_rx_freq>";
  config_stream << "			<last_tx_freq>162.3</last_tx_freq>";
  config_stream << "			<rx_antenna_choice>RX2</rx_antenna_choice>";
  config_stream << "			<enable_transmit>false</enable_transmit>";
  config_stream << "			<default_mode>NBFM</default_mode>";
  config_stream << "			<band_id>4</band_id>";
  config_stream << "			<af>";
  config_stream << "				<gain>13</gain>";
  config_stream << "				<bw>3</bw>";
  config_stream << "			</af>";
  config_stream << "			<rf>";
  config_stream << "				<gain>209</gain>";
  config_stream << "			</rf>";
  config_stream << "			<tx>";
  config_stream << "				<outpower>21.4</outpower>";
  config_stream << "				<rx_locked>false</rx_locked>";
  config_stream << "			</tx>";
  config_stream << "			<transverter_mode>false</transverter_mode>";
  config_stream << "			<transverter_local_lo>false</transverter_local_lo>";
  config_stream << "		</band>";
  config_stream << "	</bands>";
  config_stream << "</SoDaRadio>";
