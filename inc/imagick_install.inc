checkphpext() {
    cecho "Check for php extensions" $boldyellow
    if [[ ! -f "${DIR_TMP}/imagick-${IMAGICKPHP_VER}.tgz" || ! -d "${DIR_TMP}/imagick-${IMAGICKPHP_VER}" ]]; then
        echo "Downloading imagick extension"
        imagickphpexttarball
    fi
    echo
}

checkimagicksys() {

    if [ ! -f /etc/yum/pluginconf.d/versionlock.conf ]; then
        yum -y install yum-plugin-versionlock
    fi
    if [ -f /etc/yum/pluginconf.d/versionlock.conf ]; then
        VERSIONLOCK_REPO=',versionlock'
    else
        VERSIONLOCK_REPO=""
    fi

    if [ -f /usr/bin/re2c ]; then
        if [[ "$(/usr/bin/re2c -v | awk '{print $2}')" != '0.14.3' ]]; then
            rerpm
        fi
    fi

    CENTOSVER=$(awk '{ print $3 }' /etc/redhat-release)
    
    if [ "$CENTOSVER" == 'release' ]; then
        CENTOSVER=$(awk '{ print $4 }' /etc/redhat-release | cut -d . -f1,2)
        if [[ "$(cat /etc/redhat-release | awk '{ print $4 }' | cut -d . -f1)" = '7' ]]; then
            CENTOS_SEVEN='7'
        fi
    fi
    
    if [[ "$(cat /etc/redhat-release | awk '{ print $3 }' | cut -d . -f1)" = '6' ]]; then
        CENTOS_SIX='6'
    fi


    if [[ "$CENTOS_SEVEN" = '7' && "$DNF_ENABLE" = [yY] ]]; then
        # yum -y -q install epel-release
        if [[ ! -f /usr/bin/dnf ]]; then
            yum -y -q install dnf
        fi
        if [ -f /etc/yum.repos.d/rpmforge.repo ]; then
            DISABLEREPO_DNF=' --disablerepo=rpmforge'
            YUMDNFBIN="dnf${DISABLEREPO_DNF}"
        else
            DISABLEREPO_DNF=""
            YUMDNFBIN='dnf'
        fi
    else
        YUMDNFBIN='yum'
    fi

    # whether to skip ImageMagick YUM updates
    if [[ "$SKIP_IMAGEMAGICK_YUM_UPDATES" != [yY] ]]; then
        cecho "Check for ImageMagicK System Updates (YUM)" $boldyellow
        if [[ "$REMIREPO_DISABLE" = [nN] ]]; then
            if [ -f /etc/yum.repos.d/remi.repo ]; then
                if [[ $(rpm -q ImageMagick6 >/dev/null 2>&1; echo $?) = '0' ]] && [[ $(rpm -q ImageMagick >/dev/null 2>&1; echo $?) != '0' ]]; then
                    # skip for initial installs to speed up install
                    if [[ "$INITIALINSTALL" != [yY] ]]; then
                        # yum clean all >/dev/null 2>&1
                        if [[ "$CENTOS_SEVEN" = '7' && "$(rpm -qa libwebp-devel)" ]]; then
                            yum -y swap libwebp-devel libwebp7-devel --enablerepo=remi
                            time $YUMDNFBIN -y install ImageMagick6-devel ImageMagick6-c++-devel --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO}
                        fi
                        time $YUMDNFBIN -y update ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs LibRaw --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO}
                        sar_call
                    fi
                else
                    if [[ "$CENTOS_SIX" = '6' ]]; then
                        # time $YUMDNFBIN -y install libwebp libwebp-devel --skip-broken
                        sar_call
                        # yum clean all >/dev/null 2>&1
                        time yum -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO} --skip-broken
                    else
                        # yum clean all >/dev/null 2>&1
                        if [[ "$(rpm -qa libwebp-devel)" ]]; then
                            yum -y swap libwebp-devel libwebp7-devel --enablerepo=remi
                        fi
                        time yum -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs LibRaw --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO}
                    fi
                fi
            elif [ ! -f /etc/yum.repos.d/remi.repo ]; then
                # for very old centmin mod installs which never had remi yum repo setup
                if [[ "$CENTOS_SIX" = '6' ]]; then
                    remisixyum
                elif [[ "$CENTOS_SEVEN" = '7' ]]; then
                    remisevenyum
                fi
                if [[ $(rpm -q ImageMagick >/dev/null 2>&1; echo $?) = '0' ]]; then
                    echo
                    echo "----------------------------------------------------------------------------------"
                    cecho "Using Remi YUM repo ImageMagicK version" $boldyellow
                    echo "----------------------------------------------------------------------------------"
                    if [[ "$CENTOS_SEVEN" = '7' ]]; then
                        if [[ "$(rpm -qa libwebp-devel)" ]]; then
                            yum -y swap libwebp-devel libwebp7-devel --enablerepo=remi
                        fi
                        time $YUMDNFBIN -y install lcms2-devel libwebp7 libwebp7-devel OpenEXR-devel ilmbase-devel libGLU-devel libGL-devel mesa-libGL mesa-libGL-devel libXxf86vm libXxf86vm-devel --enablerepo=remi${DISABLEREPO_DNF}
                    else
                        time $YUMDNFBIN -y install lcms2-devel libwebp libwebp-devel OpenEXR-devel ilmbase-devel libGLU-devel libGL-devel mesa-libGL mesa-libGL-devel libXxf86vm libXxf86vm-devel --enablerepo=remi${DISABLEREPO_DNF}
                    fi
                    sar_call
                    yum -y remove ImageMagick${DISABLEREPO_DNF}
                    sar_call                  
                    if [[ "$CENTOS_SIX" = '6' ]]; then
                        # time $YUMDNFBIN -y install libwebp libwebp-devel --skip-broken
                        time $YUMDNFBIN -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO} --skip-broken
                        sar_call
                    else
                        if [[ "$(rpm -qa libwebp-devel)" ]]; then
                            yum -y swap libwebp-devel libwebp7-devel --enablerepo=remi
                        fi
                        time $YUMDNFBIN -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO}
                        sar_call
                    fi
                    echo
                else
                    # if ImageMagick doesn't exist
                    if [ ! -f /etc/yum.repos.d/remi.repo ]; then
                        # for very old centmin mod installs which never had remi yum repo setup
                        if [[ "$CENTOS_SIX" = '6' ]]; then
                            remisixyum
                        elif [[ "$CENTOS_SEVEN" = '7' ]]; then
                            remisevenyum
                        fi
                    fi
                    time $YUMDNFBIN -y install lcms2-devel libwebp libwebp-devel OpenEXR-devel ilmbase-devel libGLU-devel libGL-devel mesa-libGL mesa-libGL-devel libXxf86vm libXxf86vm-devel --enablerepo=remi${DISABLEREPO_DNF}
                    sar_call
                    if [[ "$CENTOS_SIX" = '6' ]]; then
                        # time $YUMDNFBIN -y install libwebp libwebp-devel --skip-broken
                        time $YUMDNFBIN -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO} --skip-broken
                        sar_call
                    else
                        if [[ "$(rpm -qa libwebp-devel)" ]]; then
                            yum -y swap libwebp-devel libwebp7-devel --enablerepo=remi
                        fi
                        time $YUMDNFBIN -y install ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs --enablerepo=remi${DISABLEREPO_DNF} --disableplugin=priorities${VERSIONLOCK_REPO}
                        sar_call
                    fi
                fi
            fi
        fi
        echo
        if [[ -f /etc/yum/pluginconf.d/versionlock.conf && -f /etc/yum.repos.d/remi.repo ]]; then
            # lock imagemagick system version from general yum updates
            # allowing only centmin.sh menu option 15 to update system
            # imagemagick version by doing an unlock of the version
            yum versionlock delete ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs LibRaw
            yum versionlock ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs LibRaw

            # temp workaround for ImageMagick6 yum update .79 build causing imagick PHP extesnion to fail
            #yum versionlock delete ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs
            #yum -y downgrade ImageMagick6-6.9.10.78-1.el7.remi ImageMagick6-c++-devel-6.9.10.78-1.el7.remi ImageMagick6-devel-6.9.10.78-1.el7.remi ImageMagick6-libs-6.9.10.78-1.el7.remi ImageMagick6-c++-6.9.10.78-1.el7.remi --enablerepo=remi
            #yum versionlock ImageMagick6 ImageMagick6-devel ImageMagick6-c++ ImageMagick6-c++-devel ImageMagick6-libs
        fi
        echo
    fi # whether to skip ImageMagick YUM updates
}

imagickinstall() {
    if [[ "$PHP_INSTALL" = [yY] ]]; then
    if [[ "$PHPIMAGICK" = [yY] ]]; then
        checkphpext
        checkimagicksys
        imagick_fixes
      # set extra cflags fro devtoolset-7 and devtoolset-8 compatibility
      # notes for users if PHP 7.0 detected
      check_devtoolset_php
      enable_devtoolset
    echo "*************************************************"
    cecho "* Installing imagick PHP Extension" $boldgreen
    echo "*************************************************"

    pwd
    echo "cd $DIR_TMP"
    cd $DIR_TMP

php -v | awk -F " " '{print $2}' | head -n1 | cut -d . -f1,2 | egrep -w '7.0||7.1|7.2|7.3|7.4|8.0'
PHPSEVEN_CHECKVER=$?
echo $PHPSEVEN_CHECKVER

if [[ "$CENTOS_SIX" -eq '6' ]]; then
    # PHP 7.3.0 + centos 6 issue https://community.centminmod.com/posts/69561/
    if [ ! -f /usr/bin/autoconf268 ]; then
        echo "yum -q -y install autoconf268"
        yum -q -y install autoconf268
    fi
    if [ -f /usr/bin/autoconf268 ]; then
        export PHP_AUTOCONF=/usr/bin/autoconf268
        export PHP_AUTOHEADER=/usr/bin/autoheader268
    fi
fi

#if [[ "$PHPMUVER" = '8.0' || "$PHPMUVER" = 'NGDEBUG' || "$PHPSEVEN_CHECKVER" = '0' ]] && [[ "$(echo $IMAGICKPHP_VER | cut -d . -f1,2 | sed -e 's|\.||')" -le '33' ]]; then
if [[ "$PHPMUVER" = '7.0' || "$PHPMUVER" = '7.1' || "$PHPMUVER" = '7.2' || "$PHPMUVER" = '7.3' || "$PHPMUVER" = '7.4' || "$PHPMUVER" = '8.0' || "$PHPMUVER" = 'NGDEBUG' || "$PHPSEVEN_CHECKVER" = '0' ]] && [[ "$(echo $IMAGICKPHP_VER | cut -d . -f1,2 | sed -e 's|\.||')" -le '33' ]]; then
    IMAGICKGITLINK='https://github.com/Imagick/imagick'
    # fallback mirror if official github is down, use gitlab mirror
    curl -${ipv_forceopt}Is --connect-timeout 5 --max-time 5 $IMAGICKGITLINK | grep 'HTTP\/' | grep '200' >/dev/null 2>&1
    IMAGICKGITCURLCHECK=$?
    if [[ "$IMAGICKGITCURLCHECK" != '0' ]]; then
        IMAGICKGITLINK='https://gitlab.com/centminmod-github-mirror/imagick.git'
    fi

    if [[ -d "imagick-php7" && -d "imagick-php7/.git" ]]; then       
        rm -rf imagick-php7
        git clone "$IMAGICKGITLINK" imagick-php7
        sar_call
    elif [[ -d "imagick-php7" && ! -d "imagick-php7/.git" ]]; then
        rm -rf imagick-php7
        git clone "$IMAGICKGITLINK" imagick-php7
        sar_call
    else
        rm -rf imagick-php7
        git clone "$IMAGICKGITLINK" imagick-php7
        sar_call
    fi
    echo
    echo "compiling imagick PHP extension for PHP 7.x ..."
    cd imagick-php7
    if [[ "$INITIALINSTALL" != [yY] ]]; then
        make clean
    fi
    /usr/local/bin/phpize
    ./configure --with-php-config=/usr/local/bin/php-config --with-imagick=/usr
    sar_call
    make${MAKETHREADS}
    sar_call
    make install
    sar_call
else    
    cd imagick-${IMAGICKPHP_VER}
    if [[ "$INITIALINSTALL" != [yY] ]]; then
        make clean
    fi
    /usr/local/bin/phpize
    ./configure --with-php-config=/usr/local/bin/php-config --with-imagick=/usr
    sar_call
    make${MAKETHREADS}
    sar_call
    make install
    sar_call
fi # php 7 or not

#######################################################
# check if imagick.so exists in php.ini

IMAGICKSOCHECK=$(grep 'extension=imagick.so' /usr/local/lib/php.ini >/dev/null 2>&1; echo $?)
IMAGICKSOCHECKB=$(grep 'extension=imagick.so' "${CONFIGSCANDIR}/imagick.ini" >/dev/null 2>&1; echo $?)

if [[ "$IMAGICKSOCHECK" = '1' || "$IMAGICKSOCHECKB" = '1' ]]; then
    echo -e "\nCopying imagick.ini > ${CONFIGSCANDIR}/imagick.ini\n"
    echo "extension=imagick.so" > "${CONFIGSCANDIR}/imagick.ini"
    echo ";imagick.skip_version_check=1" >> "${CONFIGSCANDIR}/imagick.ini"
fi #check if imagick.so exists in php.ini

    cmservice php-fpm restart
    echo "*************************************************"
    cecho "* imagick PHP installed" $boldgreen
    echo "*************************************************"

    fi
    fi # PHP_INSTALL=y
}