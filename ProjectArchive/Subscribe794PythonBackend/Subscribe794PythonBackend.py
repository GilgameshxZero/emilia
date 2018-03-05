from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.chrome.options import Options
import time
import sys

emadd = sys.argv[1]
#emadd = 'email=fontetheorigin@gmail.com'
emadd = emadd.split('=', 1)[1]

chrome_options = Options()
chrome_options.add_argument('--disable-notifications')
driver = webdriver.Chrome('chromedriver.exe', chrome_options=chrome_options)

driver.get('https://mail.cock.li/')
time.sleep(5)
driver.find_element_by_name('_user').send_keys('----')
driver.find_element_by_name('_pass').send_keys('----')
driver.find_element_by_id('rcmloginsubmit').click()
time.sleep(5)

driver.get('https://mail.cock.li/?_task=mail&_action=compose')
time.sleep(5)
driver.find_element_by_name('_to').send_keys(emadd)
driver.find_element_by_id('bcc-link').click()
driver.find_element_by_name('_bcc').send_keys('yangawesome@gmail.com')
driver.find_element_by_name('_subject').send_keys('You\'ve been subscribed to the Emilia-tan! newsletter!')
el = driver.find_element_by_name('editorSelector')
for option in el.find_elements_by_tag_name('option'):
    if option.text == 'HTML':
        option.click()
        break
time.sleep(5)
driver.switch_to_frame(driver.find_element_by_id('composebody_ifr'))
driver.find_element_by_id('tinymce').send_keys('Thank you!')
driver.find_element_by_id('tinymce').send_keys(Keys.RETURN)
driver.switch_to_default_content()
driver.find_element_by_css_selector('div[aria-label="Insert/edit image"]').click()
time.sleep(1)
driver.find_element_by_id('mceu_43-inp').send_keys('http://www.emilia-tan.com/emilia_thank_you.jpg')
time.sleep(5)
driver.find_element_by_id('mceu_43-inp').send_keys(Keys.RETURN)
time.sleep(1)
driver.find_element_by_css_selector('a[title="Send message"]').click()
time.sleep(5)

# clean up
driver.get('https://www.google.com/') # to give window time to close
driver.quit()