opencv_rpi_install()
{
	sudo apt-get install build-essential cmake pkg-config
	sudo apt-get install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
	sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
	sudo apt-get install libxvidcore-dev libx264-dev
	sudo apt-get install libgtk2.0-dev
	sudo apt-get install libatlas-base-dev gfortran
	sudo apt-get install python2.7-dev python3-dev
	cd ~
	wget -O opencv.zip https://github.com/Itseez/opencv/archive/3.1.0.zip
	unzip opencv.zip
	wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/3.1.0.zip
	unzip opencv_contrib.zip
	wget https://bootstrap.pypa.io/get-pip.py
	sudo python get-pip.py
	sudo pip install virtualenv virtualenvwrapper
	sudo rm -rf ~/.cache/pip
	# virtualenv and virtualenvwrapper
	export WORKON_HOME=$HOME/.virtualenvs
	source /usr/local/bin/virtualenvwrapper.sh
	echo -e "\n# virtualenv and virtualenvwrapper" >> ~/.profile
	echo "export WORKON_HOME=$HOME/.virtualenvs" >> ~/.profile
	echo "source /usr/local/bin/virtualenvwrapper.sh" >> ~/.profile
	source ~/.profile
	mkvirtualenv cv -p python2
	mkvirtualenv cv -p python3
	source ~/.profile
	workon cv
	pip install numpy
	cd ~/opencv-3.1.0/
	mkdir build
	cd build
	cmake -D CMAKE_BUILD_TYPE=RELEASE \
    		-D CMAKE_INSTALL_PREFIX=/usr/local \
    		-D INSTALL_PYTHON_EXAMPLES=ON \
    		-D OPENCV_EXTRA_MODULES_PATH=~/opencv_contrib-3.1.0/modules \
    		-D BUILD_EXAMPLES=ON ..
		-D WITH_CUDA=OFF
	make -j4
	make clean
	make
	sudo make install
	sudo ldconfig
	ls -l /usr/local/lib/python2.7/site-packages/
	total 1852
	-rw-r--r-- 1 root staff 1895772 Mar 20 20:00 cv2.so
	cd ~/.virtualenvs/cv/lib/python2.7/site-packages/
	ln -s /usr/local/lib/python2.7/site-packages/cv2.so cv2.so
	ls -l /usr/local/lib/python3.4/site-packages/
	total 1852
	-rw-r--r-- 1 root staff 1895932 Mar 20 21:51 cv2.cpython-34m.so
	cd /usr/local/lib/python3.4/site-packages/
	sudo mv cv2.cpython-34m.so cv2.so
	cd ~/.virtualenvs/cv/lib/python3.4/site-packages/
	ln -s /usr/local/lib/python3.4/site-packages/cv2.so cv2.so
	 source ~/.profile 
	workon cv
	rm -rf opencv-3.1.0 opencv_contrib-3.1.0
}

opencv_rpi_install
