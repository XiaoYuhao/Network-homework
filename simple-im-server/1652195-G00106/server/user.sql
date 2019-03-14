/*
 Navicat Premium Data Transfer

 Source Server         : MySQL
 Source Server Type    : MySQL
 Source Server Version : 80012
 Source Host           : localhost:3306
 Source Schema         : user

 Target Server Type    : MySQL
 Target Server Version : 80012
 File Encoding         : 65001

 Date: 23/12/2018 15:29:56
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for CONFIG
-- ----------------------------
DROP TABLE IF EXISTS `CONFIG`;
CREATE TABLE `CONFIG` (
  `size` int(11) DEFAULT NULL,
  `font` int(11) DEFAULT NULL,
  `color` int(11) DEFAULT NULL,
  `id` int(11) NOT NULL,
  `traceback` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of CONFIG
-- ----------------------------
BEGIN;
INSERT INTO `CONFIG` VALUES (15, 2, 2, 1, 100);
INSERT INTO `CONFIG` VALUES (13, 2, 2, 2, 150);
INSERT INTO `CONFIG` VALUES (15, 1, 3, 3, 100);
INSERT INTO `CONFIG` VALUES (16, 1, 4, 4, 50);
INSERT INTO `CONFIG` VALUES (18, 2, 6, 5, 40);
INSERT INTO `CONFIG` VALUES (15, 1, 5, 6, 100);
COMMIT;

-- ----------------------------
-- Table structure for HISTORY
-- ----------------------------
DROP TABLE IF EXISTS `HISTORY`;
CREATE TABLE `HISTORY` (
  `id1` int(11) NOT NULL,
  `id2` int(11) NOT NULL,
  `time` datetime(6) NOT NULL,
  `content` varchar(1020) DEFAULT NULL,
  PRIMARY KEY (`id1`,`id2`,`time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of HISTORY
-- ----------------------------
BEGIN;
INSERT INTO `HISTORY` VALUES (1, 1, '2018-12-23 15:24:24.000000', 'hello');
INSERT INTO `HISTORY` VALUES (1, 1, '2018-12-23 15:25:47.000000', 'jeklj');
INSERT INTO `HISTORY` VALUES (1, 1, '2018-12-23 15:25:56.000000', 'asdqjk');
INSERT INTO `HISTORY` VALUES (1, 1, '2018-12-23 15:26:14.000000', 'dasjdkladwqeqw');
COMMIT;

-- ----------------------------
-- Table structure for USER
-- ----------------------------
DROP TABLE IF EXISTS `USER`;
CREATE TABLE `USER` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(30) NOT NULL,
  `password` varchar(32) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL DEFAULT '123456',
  `regtime` datetime(6) DEFAULT NULL,
  `logintime` datetime(6) DEFAULT NULL,
  `passwordvalid` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of USER
-- ----------------------------
BEGIN;
INSERT INTO `USER` VALUES (1, 'simon', 'e10adc3949ba59abbe56e057f20f883e', NULL, '2018-12-23 15:26:52.000000', 1);
INSERT INTO `USER` VALUES (2, '41', 'e10adc3949ba59abbe56e057f20f883e', NULL, NULL, 0);
INSERT INTO `USER` VALUES (3, 'hello', 'e10adc3949ba59abbe56e057f20f883e', NULL, NULL, 0);
INSERT INTO `USER` VALUES (4, 'test', 'e10adc3949ba59abbe56e057f20f883e', NULL, '2018-12-23 14:45:31.000000', 1);
INSERT INTO `USER` VALUES (5, 'simon0628', 'e10adc3949ba59abbe56e057f20f883e', NULL, NULL, 0);
INSERT INTO `USER` VALUES (6, 'my', 'c33367701511b4f6020ec61ded352059', NULL, '2018-12-23 15:09:56.000000', 1);
COMMIT;

SET FOREIGN_KEY_CHECKS = 1;
