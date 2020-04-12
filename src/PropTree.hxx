/*
  Copyright (c) 2015, Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROP_TREE_HDR
#define PROP_TREE_HDR

#include <string>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/property_tree.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <boost/property_tree/exceptions.hpp>

namespace SoDa
{
/** 
   * @brief PropTree class encapsulates the USRP property tree 
   * functions to allow better trap and error recovery management. 
   */
class PropTree
{
public:
  /**
     * Constructor -- build a property tree widget rooted at the first motherboard in this multi-usrp
     * 
     * @param usrp to which USRP unit is this connected?
     * @param requester name of object that is creating this tree
     * @param mb_index which motherboard are we interested in? 
     *
     */
  PropTree(const uhd::usrp::multi_usrp::sptr usrp, const std::string &requester, int mb_index = 0)
  {
    client_name = requester;
    try
    {
      mb0_name = usrp->get_device()->get_tree()->list("/mboards").at(mb_index);
    }
    catch (std::runtime_error e)
    {
      complain((boost::format("Exception looking for motherboard %d") % mb_index).str(), e, false);
    }

    // the object is to create a tree rooted at the first motherboard.
    fqpn = "/mboards/" + mb0_name;
    try
    {
      tree = usrp->get_device()->get_tree()->subtree(fqpn);
    }
    catch (std::runtime_error e)
    {
      complain("Exception while creating initial motherboard property tree", fqpn, e, false);
    }
  }

  /**
     * Constructor -- build a property tree widget that is a subtree of the mboard property tree.
     * 
     * @param ptree pointer to ancestor property tree
     * @param path path to this property tree. 
     *
     */
  PropTree(PropTree *ptree, const std::string &path)
  {
    init(ptree, path);
  }

  /**
     * Constructor -- build a property tree widget that is a subtree of the mboard property tree.
     * 
     * @param ptree ancestor property tree
     * @param path path to this property tree. 
     *
     */
  PropTree(PropTree &ptree, const std::string &path)
  {
    init(&ptree, path);
  }

  /** 
     * @brief does the property tree have this property name as a child? 
     * @param propname the name of a property
     * @returns true if the property is found. 
     */
  bool hasProperty(const std::string &propname)
  {
    bool ret = false;
    try
    {
      std::vector<std::string> pslis = tree->list("");
      ret = std::find(pslis.begin(), pslis.end(), propname) != pslis.end();
    }
    catch (std::runtime_error e)
    {
      complain("Exception while looking for property", fqpn + "/" + propname, e, false);
    }
    return ret;
  }

  std::vector<std::string> getPropNames(const std::string &path = std::string(""))
  {
    return tree->list(path);
  }

  std::string getStringProp(const std::string &propname, const std::string defval = std::string("None"))
  {
    return getProperty<std::string>(propname, defval);
  }

  int getIntProp(const std::string &propname, const int defval = 0)
  {
    return getProperty<int>(propname, defval);
  }

  int getDoubleProp(const std::string &propname, const double defval = 0.0)
  {
    return getProperty<double>(propname, defval);
  }

  bool getBoolProp(const std::string &propname, const bool defval = false)
  {
    return getProperty<bool>(propname, defval);
  }

  void setStringProp(const std::string &propname, const std::string val = std::string("None"))
  {
    setProperty<std::string>(propname, val);
  }

  void setIntProp(const std::string &propname, const int val = 0)
  {
    setProperty<int>(propname, val);
  }

  void setDoubleProp(const std::string &propname, const double val = 0.0)
  {
    setProperty<double>(propname, val);
  }

  void setBoolProp(const std::string &propname, const bool val = false)
  {
    setProperty<bool>(propname, val);
  }

  template <typename T>
  T getProperty(const std::string &propname, const T defval)
  {
    T ret = defval;
    std::string path = fqpn + "/" + propname;
    try
    {
      ret = tree->access<T>(propname).get();
    }
    catch (std::runtime_error e)
    {
      complain("getProperty Unknown exception -- unknown property", path, e);
    }
    return ret;
  }

  template <typename T>
  T getProperty(const std::string &propname)
  {
    T ret;
    std::string path = fqpn + "/" + propname;
    try
    {
      ret = tree->access<T>(propname).get();
    }
    catch (std::runtime_error e)
    {
      complain("getProperty Unknown exception -- unknown property", path, e, false);
    }
    return ret;
  }

  template <typename T>
  void setProperty(const std::string &propname, const T val)
  {
    std::string path = fqpn + "/" + propname;
    try
    {
      tree->access<T>(propname).set(val);
    }
    catch (std::runtime_error e)
    {
      complain("setProperty Unknown exception -- unknown property", path, e);
    }
  }

private:
  void init(PropTree *ptree, const std::string &path)
  {
    client_name = ptree->client_name;
    try
    {
      tree = ptree->tree->subtree(path);
    }
    catch (std::runtime_error e)
    {
      complain("Exception while creating a subtree", path, e, false);
    }

    fqpn = ptree->fqpn + "/" + path;
  }

  void complain(const std::string &explain_string,
                std::runtime_error &e,
                bool continue_p = true)
  {
    std::cerr << boost::format("PropTree encountered %s\n\tRequester: %s\n\tException: [%s]\n\t%s\n") % explain_string % client_name % e.what() % (continue_p ? "Thread will continue" : "Thread will terminate");
    if (!continue_p)
      exit(-1);
  }

  void complain(const std::string &explain_string,
                const std::string &path,
                std::runtime_error &e,
                bool continue_p = true)
  {
    std::cerr << boost::format("PropTree encountered %s\n\twhile looking for path [%s]\n\tRequester: %s\n\tException: [%s]\n\t%s\n") % explain_string % path % client_name % e.what() % (continue_p ? "Thread will continue" : "Thread will terminate");

    if (!continue_p)
      exit(-1);
  }

  std::string client_name; ///< the name of the runtime object/thread that created this tree.
  std::string fqpn;
  uhd::property_tree::sptr tree;
  std::string mb0_name;
};
} // namespace SoDa
#endif
