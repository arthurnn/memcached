FROM ubuntu:precise
ENV DEBIAN_FRONTEND noninteractive

# Install packages for building ruby
RUN apt-get update
RUN apt-get install -y --force-yes build-essential pkg-config curl git
RUN apt-get install -y --force-yes zlib1g-dev libssl-dev libreadline-dev libyaml-dev libxml2-dev libxslt-dev
RUN apt-get install -y libsasl2-dev memcached
RUN apt-get clean

# Setup user
RUN useradd -ms /bin/bash runner

# Install rbenv and ruby-build
RUN git clone https://github.com/sstephenson/rbenv.git /home/runner/.rbenv
RUN git clone https://github.com/sstephenson/ruby-build.git /home/runner/.rbenv/plugins/ruby-build
RUN ["/bin/bash", "-c", "/home/runner/.rbenv/plugins/ruby-build/install.sh"]
RUN echo 'eval "$(rbenv init -)"' >> /home/runner/.bashrc
RUN echo 'eval "$(rbenv init -)"' >> /etc/profile

RUN chown -R runner /home/runner
USER runner

ENV PATH /home/runner/.rbenv/bin:$PATH

# install ruby
RUN ["/bin/bash", "-c", "rbenv install 2.1.7"]
RUN ["/bin/bash", "-c", "rbenv global 2.1.7"]
ENV PATH /home/runner/.rbenv/shims:$PATH
RUN gem install bundler

ENV APP_HOME /home/runner/src
WORKDIR $APP_HOME

ADD Gemfile* $APP_HOME/
ADD memcached.gemspec $APP_HOME/
RUN bundle install

ADD . $APP_HOME

USER root
RUN chown -R runner /home/runner
USER runner


CMD bundle exec ruby -Ilib:test test/setup.rb && bundle exec rake
